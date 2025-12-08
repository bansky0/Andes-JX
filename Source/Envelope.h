/*
  ==============================================================================

    Envelope.h
    Created: 7 Dec 2025 12:36:44pm
    Author:  Jhonatan

  ==============================================================================
*/

#pragma once

const float SILENCE = 0.0001f;

class Envelope
{
private:
    float target;
    float multiplier;
public:
    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;
    
    float level;
    
    float nextValue()
    {
        level = multiplier * (level - target) + target;
        if (level + target > 3.0f) {
            multiplier = decayMultiplier;
            target = sustainLevel;
        }
        return level;
    }

    inline bool isActive() const
    {
        return level > SILENCE;
    }

    inline bool isInAttack() const
    {
        return target >= 2.0f;
    }

    void attack()
    {
        level += SILENCE + SILENCE;
        target = 2.0f;
        multiplier = attackMultiplier;
    }

    void release()
    {
        target = 0.0f;
        multiplier = releaseMultiplier;
    }

    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }
};