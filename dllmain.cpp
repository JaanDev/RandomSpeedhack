#include "pch.h"
#include <cocos2d.h>
#include "MenuOptions.h"
#include "minhook/include/MinHook.h"
#include "GameManager.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace cocos2d;

namespace SpeedhackAudio {
    void* channel;
    float speed;
    bool initialized = false;

    // setfrequency
    // setvolume

    void* (__stdcall* setVolume)(void* t_channel, float volume);
    void* (__stdcall* setFrequency)(void* t_channel, float frequency);

    void* __stdcall AhsjkabdjhadbjJHDSJ(void* t_channel, float volume) {
        channel = t_channel;

        if (speed != 1.f) {
            setFrequency(channel, speed);

        }
        return setVolume(channel, volume);
    }

    void init() {
        if (initialized)
            return;

        setFrequency = (decltype(setFrequency))GetProcAddress(GetModuleHandle(L"fmod.dll"), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
        DWORD hkAddr = (DWORD)GetProcAddress(GetModuleHandle(L"fmod.dll"), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");

        MH_CreateHook(
            (PVOID)hkAddr,
            AhsjkabdjhadbjJHDSJ,
            (PVOID*)&setVolume
        );

        speed = 1.f;
        initialized = true;
    }

    void set(float frequency) {
        if (!initialized)
            init();

        if (channel == nullptr)
            return;

        speed = frequency;
        setFrequency(channel, frequency);
    }
}

namespace PlayLayer {
    void(__thiscall* update)(CCLayer* self, float dt);
    void __fastcall updateHook(CCLayer* self, void*, float dt);

    void(__thiscall* resetLevel)(CCLayer* self);
    void __fastcall resetLevelHook(CCLayer* self, void*);

    void(__thiscall* onQuit)(CCLayer* self);
    void __fastcall onQuitHook(CCLayer* self, void*);

    bool(__thiscall* init)(CCLayer* self, void* level);
    bool __fastcall initHook(CCLayer* self, void*, void* level);

    void mem_init();

    int secondsLeft = 5;
    int currentSeconds = time(0);

    const int TAG = 232189;

    const char* key = "random-speedhack";

    std::string IntToString(int N)
    {
        std::ostringstream ss("");
        ss << N;
        return ss.str();
    }

    float RandomFloat(float a, float b) {
        float random = ((float)rand()) / (float)RAND_MAX;
        float diff = b - a;
        float r = random * diff;
        return a + r;
    }

    int RandomInt(int a, int b) {
        float random = ((float)rand()) / (float)RAND_MAX;
        float diff = b - a;
        float r = random * diff;
        return (int)(a + r);
    }

    void __fastcall PlayLayer::resetLevelHook(CCLayer* self, void*) {
        void* gm = GameManager::getSharedState();

        if (GameManager::getGameVariable(gm, key))
            currentSeconds = time(0);

        PlayLayer::resetLevel(self);
    }

    void __fastcall PlayLayer::onQuitHook(CCLayer* self, void*) {
        void* gm = GameManager::getSharedState();

        if (GameManager::getGameVariable(gm, key)) {
            CCDirector* director = CCDirector::sharedDirector();

            CCScheduler* s = director->getScheduler();

            s->setTimeScale(1.f);
            SpeedhackAudio::set(1);
        }

        PlayLayer::onQuit(self);
    }

    bool __fastcall PlayLayer::initHook(CCLayer* self, void*, void* level) {
        bool result = PlayLayer::init(self, level);

        void* gm = GameManager::getSharedState();

        if (GameManager::getGameVariable(gm, key)) {
            SpeedhackAudio::set(1);

            currentSeconds = time(0);
            secondsLeft = 5;

            self->removeChildByTag(TAG);

            int timel = (currentSeconds + secondsLeft) - time(0);
            std::string secondsLeft = IntToString(timel);
            std::string txt1 = std::string("Time left: ") + secondsLeft;

            const char* txt = txt1.c_str();

            CCLabelBMFont* text = CCLabelBMFont::create(txt, "goldFont-uhd.fnt");

            text->setTag(TAG);

            text->setAnchorPoint({ 0, 0 });

            text->setPosition({ 10, 10 });

            self->addChild(text, 999);
        }

        return result;
    }

    void __fastcall PlayLayer::updateHook(CCLayer* self, void*, float dt) {
        void* gm = GameManager::getSharedState();

        if (GameManager::getGameVariable(gm, key)) {
            CCLabelBMFont* text = (CCLabelBMFont*)self->getChildByTag(TAG);

            int timel = (currentSeconds + secondsLeft) - time(0);

            if (timel < 0) {
                std::string txt1 = std::string("Please re-enter the level for the counter to work properly!");

                const char* txt = txt1.c_str();

                text->setCString(txt);

                text->setScale(.5f);
            }

            std::string secondsL = IntToString(timel);
            std::string txt1 = std::string("Time left: ") + secondsL;

            const char* txt = txt1.c_str();

            text->setCString(txt);

            if ((currentSeconds + secondsLeft) == time(0)) {

                float newSpeed = RandomFloat(0.1f, 2.5f);

                newSpeed = round(newSpeed * 10) / 10;

                CCDirector* director = CCDirector::sharedDirector();

                CCScheduler* s = director->getScheduler();

                s->setTimeScale(newSpeed);

                // fmod

                SpeedhackAudio::set(newSpeed);

                // time

                secondsLeft = RandomInt(5, 10);

                currentSeconds = time(0);
            }
        }

        PlayLayer::update(self, dt);
    }

    void PlayLayer::mem_init() {
        // Hook
        size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

        MH_CreateHook(
            (PVOID)(base + 0x2029C0),
            PlayLayer::updateHook,
            (PVOID*)&PlayLayer::update);

        MH_CreateHook(
            (PVOID)(base + 0x20BF00),
            PlayLayer::resetLevelHook,
            (PVOID*)&PlayLayer::resetLevel);

        MH_CreateHook(
            (PVOID)(base + 0x20D810),
            PlayLayer::onQuitHook,
            (PVOID*)&PlayLayer::onQuit);

        MH_CreateHook(
            (PVOID)(base + 0x01FB780),
            PlayLayer::initHook,
            (PVOID*)&PlayLayer::init);
    }
}

DWORD WINAPI my_thread(void* hModule) {
    MH_Initialize();

    GameManager::mem_init();
    MenuOptions::mem_init();
    SpeedhackAudio::init();
    PlayLayer::mem_init();

    MH_EnableHook(MH_ALL_HOOKS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

