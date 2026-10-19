// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/System/Library.hpp"
#include <alsa/asoundlib.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define ZY_ALSA(Name) inline decltype(& ::Name) zy##Name = nullptr;
#include "ALSA.inl"
#undef ZY_ALSA

// Every call keeps its ALSA name, which is a macro over the loader's own pointer, so nothing links against ALSA.
#define snd_device_name_free_hint zysnd_device_name_free_hint
#define snd_device_name_get_hint  zysnd_device_name_get_hint
#define snd_device_name_hint      zysnd_device_name_hint
#define snd_pcm_close             zysnd_pcm_close
#define snd_pcm_drop              zysnd_pcm_drop
#define snd_pcm_open              zysnd_pcm_open
#define snd_pcm_prepare           zysnd_pcm_prepare
#define snd_pcm_recover           zysnd_pcm_recover
#define snd_pcm_set_params        zysnd_pcm_set_params
#define snd_pcm_writei            zysnd_pcm_writei
#define snd_strerror              zysnd_strerror

namespace ZyAudio
{
    /// \brief Opens the system's ALSA library, which the audio driver plays through.
    class ALSA final
    {
    public:

        /// \brief Opens the library on the first call, and answers every later call with how that one went.
        ///
        /// \return `true` when the library opened and every entry point was found, `false` otherwise.
        ZY_INLINE static Bool Open()
        {
            static const Bool Opened = Load();
            return Opened;
        }

    private:

        /// \brief Opens the library, which stays open until the process ends, and takes every entry point out of it.
        ///
        /// \return `true` when the library opened and every entry point was found, `false` otherwise.
        ZY_INLINE static Bool Load()
        {
            static Library Asound("libasound.so.2");

            Bool Complete = Asound.IsOpen();

#define ZY_ALSA(Name)                                                 \
            zy##Name = Asound.GetFunction<decltype(zy##Name)>(#Name); \
            Complete = Complete && (zy##Name != nullptr);
#include "ALSA.inl"
#undef ZY_ALSA

            if (!Complete)
            {
                LOG_E("Audio: Failed to load ALSA (libasound.so.2)");
            }
            return Complete;
        }
    };
}