/*
  ==============================================================================

    Voice.h
    Created: 10 Nov 2025 6:45:29pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

struct Voice
{
    int note;
    int velocity;
    void reset()
    {
        note = 0;
        velocity = 0;
    }
};