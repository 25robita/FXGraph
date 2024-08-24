/*
  ==============================================================================

    Envelope.cpp
    Created: 10 Aug 2024 9:34:29am
    Author:  School

  ==============================================================================
*/

#include "Envelope.h"
#include "JuceHeader.h"

Envelope::Envelope()
{
    Envelope(0.035, 0.035, 441000.0);
}

Envelope::Envelope(float msAttack_, float msRelease_, float blockRate_)
{
    jassert(msAttack_ > 0.0);
    jassert(msRelease_ > 0.0);
    jassert(blockRate_ > 0.0);
    
    msAttack = msAttack_;
    msRelease = msRelease_;
    blockRate = blockRate_;

    updateCoef();
}

void Envelope::updateCoef()
{
    coefA = exp( -1000.0 / ( msAttack * blockRate) );
    coefR = exp( -1000.0 / ( msRelease * blockRate) );
}

void Envelope::setMsAttack(float ms_)
{
    jassert(ms_ > 0.0);
    msAttack = ms_;
    updateCoef();
}

void Envelope::setMsRelease(float ms_)
{
    jassert(ms_ > 0.0);
    msRelease = ms_;
    updateCoef();
}

void Envelope::setBlockRate(float blockRate_)
{
    jassert(blockRate_ > 0.0);
    blockRate = blockRate_;
    updateCoef();
}

void Envelope::run(float inpt, float &curr)
{
    // if increasing, then attack, else release
    curr = inpt + (inpt > curr ? coefA : coefR) * (curr - inpt);
}
