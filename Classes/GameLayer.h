#ifndef __GAMELAYER_H__
#define __GAMELAYER_H__

#define GOAL_WIDTH 400

#include <cocos2d.h>
#include "GameSprite.h"

USING_NS_CC;

class GameLayer: public Layer
{
  public:
    GameLayer();
    virtual ~GameLayer();
    virtual bool init();

    static Scene* scene();

    CREATE_FUNC(GameLayer);

    void onTouchesBegan(const std::vector<Touch*> &touches, Event* event);
    void onTouchesMoved(const std::vector<Touch*> &touches, Event* event);
    void onTouchesEnded(const std::vector<Touch*> &touches, Event* event);

    void update(float dt);

  private:
    GameSprite* _player1;
    GameSprite* _player2;
    GameSprite* _ball;

    Vector<GameSprite*> _players;
    Label* _player1ScoreLabel;
    Label* _player2ScoreLabel;

    Size _screenSize;

    int _player1Score;
    int _player2Score;

    void playerScore(int player);
};

#endif // __GAMELAYER_H__
