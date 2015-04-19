#include <GameLayer.h>
#include <SimpleAudioEngine.h>

GameLayer::GameLayer()
{
}

GameLayer::~GameLayer()
{
}

Scene* GameLayer::scene()
{
    auto scene = Scene::create();

    auto layer = GameLayer::create();

    scene->addChild(layer);

    return scene;
}

bool GameLayer::init()
{
  if (!Layer::init()) {
    return false;
  }

  _players = Vector<GameSprite*>(2);
  _player1Score = 0;
  _player2Score = 0;
  _screenSize = Director::getInstance()->getWinSize();

  auto court = Sprite::create("court.png");
  court->setPosition(Vec2(_screenSize.width * 0.5,
                          _screenSize.height * 0.5));
  this->addChild(court);

  _player1 = GameSprite::gameSpriteWithFile("mallet.png");
  _player1->setPosition(Vec2(_screenSize.width * 0.5,
                             _player1->radius() * 2));
  _players.pushBack(_player1);
  this->addChild(_player1);

  _player2 = GameSprite::gameSpriteWithFile("mallet.png");
  _player2->setPosition(Vec2(_screenSize.width * 0.5,
                             _screenSize.height - _player2->radius() * 2));
  _players.pushBack(_player2);
  this->addChild(_player2);

  _ball = GameSprite::gameSpriteWithFile("puck.png");
  _ball->setPosition(Vec2(_screenSize.width * 0.5,
                          _screenSize.height * 0.5 - 2 * _ball->radius()));
  this->addChild(_ball);

  // create TTF labels
  _player1ScoreLabel = Label::createWithTTF("0", "fonts/Arial.ttf", 60);
  _player1ScoreLabel->setPosition(Vec2(_screenSize.width - 60,
                                       _screenSize.height * 0.5 - 80));
  _player1ScoreLabel->setRotation(90);
  this->addChild(_player1ScoreLabel);

  _player2ScoreLabel = Label::createWithTTF("0", "fonts/Arial.ttf", 60);
  _player2ScoreLabel->setPosition(Vec2(_screenSize.width - 60,
                                       _screenSize.height * 0.5 + 80));
  _player2ScoreLabel->setRotation(90);
  this->addChild(_player2ScoreLabel);

  // multitouch events
  auto listener = EventListenerTouchAllAtOnce::create();
  listener->onTouchesBegan = CC_CALLBACK_2(GameLayer::onTouchesBegan, this);
  listener->onTouchesMoved = CC_CALLBACK_2(GameLayer::onTouchesMoved, this);
  listener->onTouchesEnded = CC_CALLBACK_2(GameLayer::onTouchesEnded, this);

  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

  this->scheduleUpdate();
  return true;
}

void GameLayer::onTouchesBegan(const std::vector<Touch*> &touches, Event* event)
{
  for(auto touch: touches) {
    if (touch != nullptr) {
      auto tap = touch->getLocation();
      for(auto player: _players) {
        if (player->boundingBox().containsPoint(tap)) {
          player->setTouch(touch);
          // TODO: should we break the loop?
        }
      }
    }
  }
}

void GameLayer::onTouchesMoved(const std::vector<Touch*> &touches, Event* event)
{
  for(auto touch: touches) {
    if (touch != nullptr) {
      auto tap = touch->getLocation();
      for(auto player: _players) {
        if (player->getTouch() != nullptr && player->getTouch() == touch) {
          Point nextPosition = tap;
          if (nextPosition.x < player->radius())
            nextPosition.x = player->radius();

          if (nextPosition.x > _screenSize.width - player->radius())
            nextPosition.x = _screenSize.width - player->radius();

          if (nextPosition.y < player->radius())
            nextPosition.y = player->radius();

          if (nextPosition.y > _screenSize.height - player->radius())
            nextPosition.y = _screenSize.height - player->radius();


          if (player->getPositionY() < _screenSize.height * 0.5f) {
            if (nextPosition.y > _screenSize.height * 0.5f - player->radius()) {
              nextPosition.y = _screenSize.height * 0.5f - player->radius();
            }
          } else {
            if (nextPosition.y < _screenSize.height * 0.5f + player->radius()) {
              nextPosition.y = _screenSize.height * 0.5f + player->radius();
            }
          }

          player->setNextPosition(nextPosition);
          player->setVector(Vec2(tap.x - player->getPositionX(),
                                 tap.y - player->getPositionY()));

        }
      }
    }
  }
}

void GameLayer::onTouchesEnded(const std::vector<Touch*> &touches, Event* event)
{
  for(auto touch: touches) {
    if (touch != nullptr) {
      for (auto player: _players) {
        if (player->getTouch() != nullptr && player->getTouch() == touch) {
          player->setTouch(nullptr);
          player->setVector(Vec2(0, 0));
        }
      }
    }
  }
}

void GameLayer::update(float dt)
{
  auto ballNextPosition = _ball->getNextPosition();
  auto ballVector = _ball->getVector();
  ballVector *= 0.98f;

  ballNextPosition.x += ballVector.x;
  ballNextPosition.y += ballVector.y;

  float squared_radii = pow(_player1->radius() - _ball->radius(), 2);
  for (auto player: _players) {
    auto playerNextPosition = player->getNextPosition();
    auto playerVector = player->getVector();

    float diffx = ballNextPosition.x - player->getPositionX();
    float diffy = ballNextPosition.y - player->getPositionY();
    float distance1 = ballNextPosition.distanceSquared(player->getPosition());
    float distance2 = playerNextPosition.distanceSquared(_ball->getPosition());

    if (distance1 <= squared_radii ||
        distance2 <= squared_radii) {
      float mag_ball = pow(ballVector.x, 2) + pow(ballVector.y, 2); // FIXME: getLenghtSq??
      float mag_player = pow(playerVector.x, 2) + pow(playerVector.y, 2);

      float force = sqrt(mag_ball + mag_player);
      float angle = atan2f(diffy, diffx);

      ballVector.x = force * cos(angle);
      ballVector.y = force * sin(angle);

      ballNextPosition.x = playerNextPosition.x + (player->radius() + _ball->radius() + force) * cos(angle);
      ballNextPosition.y = playerNextPosition.y + (player->radius() + _ball->radius() + force) * sin(angle);

      CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hit.wav");
    }
  }

  // hit left wall
  if (ballNextPosition.x < _ball->radius()) {
    ballNextPosition.x = _ball->radius();
    ballVector.x *= -0.8f;
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hit.wav");
  }

  // hit right wall
  if (ballNextPosition.x > _screenSize.width - _ball->radius()) {
    ballNextPosition.x = _screenSize.width - _ball->radius();
    ballVector.x *= -0.8f;
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hit.wav");
  }

  // check for point
  if (ballNextPosition.y > _screenSize.height - _ball->radius()) {
    // if the ball hits outside the goal area
    if (_ball->getPosition().x < _screenSize.width * 0.5f - GOAL_WIDTH * 0.5f ||
        _ball->getPosition().x > _screenSize.width * 0.5f + GOAL_WIDTH * 0.5f) {
      ballNextPosition.y = _screenSize.height - _ball->radius();
      ballVector.y *= -0.8f;
      CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hit.wav");
    }
  }
  if (ballNextPosition.y > _ball->radius()) {
    // if the ball hits outside the goal area
    if (_ball->getPosition().x < _screenSize.width * 0.5f - GOAL_WIDTH * 0.5f ||
        _ball->getPosition().x > _screenSize.width * 0.5f + GOAL_WIDTH * 0.5f) {
      ballNextPosition.y = _ball->radius();
      ballVector.y *= -0.8f;
      CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hit.wav");
    }
  }

  _ball->setVector(ballVector);
  _ball->setNextPosition(ballNextPosition);

  if (ballNextPosition.y < _ball->radius() * 2) {
    this->playerScore(2);
  }

  if (ballNextPosition.y > _screenSize.height +  _ball->radius() * 2) {
    this->playerScore(1);
  }

  _player1->setPosition(_player1->getNextPosition());
  _player2->setPosition(_player2->getNextPosition());
  _ball->setPosition(_ball->getNextPosition());
}

void GameLayer::playerScore(int player)
{
  CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("score.wav");
  _ball->setVector(Vec2(0, 0));

  char score_buffer[10];
  if (player == 1) {
    _player1Score++;
    _player1ScoreLabel->setString(std::to_string(_player1Score));
    _ball->setNextPosition(Vec2(_screenSize.width * 0.5f, _screenSize.height * 0.5f +
                                _ball->radius() * 2));
  } else {
    _player2Score++;
    _player2ScoreLabel->setString(std::to_string(_player2Score));
    _ball->setNextPosition(Vec2(_screenSize.width * 0.5f, _screenSize.height * 0.5f -
                                _ball->radius() * 2));
  }

  _player1->setPosition(Vec2(_screenSize.width * 0.5f, _player1->radius() * 2));
  _player2->setPosition(Vec2(_screenSize.width * 0.5f,
                             _screenSize.height - _player2->radius() * 2));
  _player1->setTouch(nullptr);
  _player2->setTouch(nullptr);
}
