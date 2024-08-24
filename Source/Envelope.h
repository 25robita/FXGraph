/*
  ==============================================================================

    Envelope.h
    Created: 10 Aug 2024 9:34:29am
    Author:  School

  ==============================================================================
*/

#pragma once


class Envelope
{
public:
    Envelope();
    Envelope(float msAttack_, float msRelease_, float blockRate_);

    void setMsAttack(float ms_);
    float getMsAttack() {return msAttack;};
    
    void setMsRelease(float ms_);
    float getMsRelease() {return msRelease;};
    
    void setBlockRate(float blockRate_);
    float getBlockRate() {return blockRate;};
    
    void run(float inpt, float &curr); // sets curr by reference
    
    
private:
    float msAttack;
    float msRelease;
    float blockRate;
    float coefA;
    float coefR;
    
    void updateCoef();
};
