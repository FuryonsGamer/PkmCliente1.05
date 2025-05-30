/*
 * Copyright (c) 2010-2017 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "effect.h"
#include "map.h"
#include "game.h"
#include <framework/core/eventdispatcher.h>
#include <framework/util/extras.h>

void Effect::setEffect(uint16 effectId)
{
    if (!g_things.isValidDatId(effectId, ThingCategoryEffect)) {
        return;
    }
    setId(effectId);
    m_animationTimer.restart();
    int duration = 0;
    if(g_game.getFeature(Otc::GameEnhancedAnimations)) {
        duration = getThingType()->getAnimator() ? getThingType()->getAnimator()->getTotalDuration() : 1000;
    } else {
        duration = EFFECT_TICKS_PER_FRAME;
        duration *= getAnimationPhases();
    }
}

void Effect::draw(const Point& dest, int offsetX, int offsetY, bool animate, LightView* lightView)
{
    if(m_id == 0)
        return;

    if(animate) {
        if(g_game.getFeature(Otc::GameEnhancedAnimations) && rawGetThingType()->getAnimator()) {
            // This requires a separate getPhaseAt method as using getPhase would make all magic effects use the same phase regardless of their appearance time
            m_animationPhase = std::max<int>(0, rawGetThingType()->getAnimator()->getPhaseAt(m_animationTimer, m_animationPhase));
        } else {
            // hack to fix some animation phases duration, currently there is no better solution
            int ticks = EFFECT_TICKS_PER_FRAME;
            if (m_id == 33) {
                ticks <<= 2;
            }

            m_animationPhase = std::max<int>(0, std::min<int>((int)(m_animationTimer.ticksElapsed() / ticks), getAnimationPhases() - 1));
        }
    }

    int xPattern = m_position.x % getNumPatternX();
    if(xPattern < 0)
        xPattern += getNumPatternX();

    int yPattern = m_position.y % getNumPatternY();
    if(yPattern < 0)
        yPattern += getNumPatternY();

    Color color = Color::white;
    color.setOpacity(g_game.getEffectOpacity());

    Point effectDisplacement = rawGetThingType()->getEffectDisplacement();
    rawGetThingType()->draw(dest + effectDisplacement, 0, xPattern, yPattern, 0, m_animationPhase, color, lightView);
}

void Effect::onAppear()
{
    m_animationTimer.restart();

    int duration = 0;
    if(g_game.getFeature(Otc::GameEnhancedAnimations)) {
        duration = getThingType()->getAnimator() ? getThingType()->getAnimator()->getTotalDuration() : 1000;
    } else {
        duration = EFFECT_TICKS_PER_FRAME;

        // hack to fix some animation phases duration, currently there is no better solution
        if(m_id == 33) {
            duration <<= 2;
        }

        duration *= getAnimationPhases();
    }

    // schedule removal
    auto self = asEffect();
    g_dispatcher.scheduleEvent([self]() { g_map.removeThing(self); }, duration);
}

void Effect::setId(uint32 id)
{
    if(!g_things.isValidDatId(id, ThingCategoryEffect))
        id = 0;
    m_id = id;
}

const ThingTypePtr& Effect::getThingType()
{
    return g_things.getThingType(m_id, ThingCategoryEffect);
}

ThingType *Effect::rawGetThingType()
{
    return g_things.rawGetThingType(m_id, ThingCategoryEffect);
}
