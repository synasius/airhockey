#ifndef __GAMESPRITE_H__
#define __GAMESPRITE_H__

#include <cocos2d.h>

USING_NS_CC;

class GameSprite: public Sprite
{
  public:
    CC_SYNTHESIZE(Vec2, _nextPosition, NextPosition);
    CC_SYNTHESIZE(Vec2, _vector, Vector);
    CC_SYNTHESIZE(Touch*, _touch, Touch);

    GameSprite();
    virtual ~GameSprite();

    virtual void setPosition(const Vec2& pos) override;
    float radius();

    static GameSprite* gameSpriteWithFile(const char* pszFileName);
};

inline float GameSprite::radius()
{
 return this->getTexture()->getContentSize().width * 0.5f;
}

#endif // __GAMESPRITE_H__
