/**
Bacsu Attila
H7RTVY
bacat
**/

#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

#include "graphics.hpp"

using namespace std;
using namespace genv;

//Instead of magic numbers and defines... :
constexpr int windowWidth{800}, windowHeight{600};
constexpr int ballRadius{10}, ballStartVelocity{4};
constexpr int paddleWidth{120}, paddleHeight{20};
constexpr int brickWidth{60}, brickHeight{20};
constexpr int brickCountX{windowWidth / (brickWidth + 10)}, brickCountY{4};
constexpr float angleCorrector{90.f}, minAngle{20.f}, maxAngle{160.f}, angle_Unit{180.f / paddleWidth};
constexpr double pi() { return atan(1)*4; }

  ///***********************************************************************\\\
 ///**************************GAMEOBJECT CLASSES************************\\\
///**************************************************************************\\\

// Superclass of all shapes in the game
struct Shape
{
protected:
        int pos_x, pos_y;
        int width, height;
public:
        //With these functions, I can handle my shapes more easier.
        float x() const { return pos_x + (width / 2); }
        float y() const { return pos_y + (height / 2); }
        float left() const { return pos_x; }
        float right() const { return pos_x + width; }
        float top() const { return pos_y; }
        float bottom() const { return pos_y + height; }
};

//Subclass Paddle. We can hit back the ball with it.
struct Paddle : public Shape
{
        Paddle() {}
        Paddle(int startX, int startY)
        {
                width = paddleWidth;
                height = paddleHeight;
                pos_x = startX - (width / 2);
                pos_y = startY - (height / 2);
        }
        void draw() const { gout << move_to(pos_x, pos_y) << color(255, 0, 0) << box(width, height); }
        void update(int X) { if(X > paddleWidth / 2 && X < windowWidth - (paddleWidth / 2)) pos_x = X - (width / 2); }
};

//Subclass Brick. We have to destroy these object with the ball.
struct Brick : public Shape
{
        bool isDestroyed;

        Brick(int X, int Y)
        {
                isDestroyed = false;
                width = brickWidth;
                height = brickHeight;
                pos_x = X - (width / 2);
                pos_y = Y - (height / 2);
        }
        void draw() const { gout << move_to(pos_x, pos_y) << color(255, 255, 0) << box(width, height); }
};

// Subclass Ball. We have to destroy bricks, with this ball.
struct Ball : public Shape
{
        bool isLost; //to handle if we can't hit back the ball with the paddle
        float velocityX{0}, velocityY{0}; //speed of the ball

        Ball() {}
        Ball(Paddle paddle)
        {
                isLost = false;
                pos_x = paddle.x();
                pos_y = paddle.y();
                width = height = ballRadius;
        }

        //move the ball with the given speed
        void move(float velocity_x, float velocity_y)
        {
                pos_x += velocity_x;
                pos_y += velocity_y;
        }

        //update the ball - this means move it, and check if hits a wall, or lose it
        void update()
        {
                move(velocityX, velocityY);

                if(left() < 0 && velocityX < 0) velocityX = -velocityX;
                else if(right() > windowWidth && velocityX > 0) velocityX = -velocityX;

                if(top() < 0 && velocityY < 0) velocityY = -velocityY;
                else if(top() > windowHeight) isLost = true;
        }

        //update the ball before gamestart -  it stays in the middle of the paddle
        void update_beforeStarted(Paddle paddle)
        {
                pos_x = paddle.x();
                pos_y = paddle.y() - paddleHeight + 3;
        }

    //draw the ball to the screen (középponttól való távolság alapján)
        void draw() const
        {
                gout << color(255, 0, 0);
                for(int i{pos_x}; i < (pos_x + ballRadius * 2); i++)
                {
                        for(int j{pos_y}; j < (pos_y + ballRadius * 2); j++)
                        {
                                if(sqrt((x() - i) * (x() - i) + (y() - j) * (y() - j)) < ballRadius*0.5) gout << move_to(i, j) << dot;
                        }
                }
        }

};

  ///***********************************************************************\\\
 ///**************************GAMEPLAY FUNCTIONS************************\\\
///**************************************************************************\\\

//Let's create a template function for intersecting test!
//I think this is the point, where C++ coming more interesting. :P
//So, I wanna make function for testing intersection between two shapes.
//And template is _for between ANY of shapes_! (Who has property methods, called right(), left(), bottom(), top().)
//It's awesome! I can be very productive with a few amount of code. Nyilván minden template fv. átalakítható függvénytúlterheléssé.
//De hála ennek, nem kell külön leírni az összes lehetséges paraméterezést... majd a fordító elintézi nekem. :)
//I'm very newie in generic programming, but I love it! :)
template<class T1, class T2> bool isIntersecting(T1& ShapeA, T2& ShapeB)
{
        return ShapeA.right() >= ShapeB.left() && ShapeA.left() <= ShapeB.right() &&
                ShapeA.top() <= ShapeB.bottom() && ShapeA.bottom() >= ShapeB.top();
}

//Test collision between paddle and ball
void testCollision(Paddle& ShapePaddle, Ball& ShapeBall)
{
        if(!isIntersecting(ShapePaddle, ShapeBall)) return;

        float whereIntersect_RelativeToPaddle{ShapePaddle.right() - ShapeBall.x()};
        float getAngle{angle_Unit * whereIntersect_RelativeToPaddle}; // remember, that 'angle_Unit' is equal to 180/paddleWidth (a 'félkör' felosztva az ütő mérete szerint)

        // Just for humanity gameplay - enélkül szépen néz ki, amikor sikerül a legszélével eltalálni a golyót és vízszintesen kezd el pattogni fal és ütő között... :)
        if(getAngle < minAngle) getAngle = minAngle;
        if(getAngle > maxAngle) getAngle = maxAngle;

        ShapeBall.velocityX = sin((getAngle + angleCorrector) * (pi() / 180.f)) * ballStartVelocity; // angleCorrector az 'elforgatott' egységkör miatt
        ShapeBall.velocityY = cos((getAngle+ angleCorrector) * (pi() / 180.f)) * ballStartVelocity; // és *(pi() / 180.f) nyilván, mert radiánban kell a fv-nek
}

//Test collision between a brick and the ball
void testCollision(Brick& ShapeBrick, Ball& ShapeBall)
{
        if(!isIntersecting(ShapeBrick, ShapeBall)) return; //return if not instersecting

        ShapeBrick.isDestroyed = true;

        //Yes, the next part of code is not too nice, but during test it worked better, than other methods
        //like testing, where intersecting the two shapes each other
        // Az alapötlet az, hogy ha a ball eltalál egy brick-et akkor egyik sebességet megváltoztatva továbbmozgatjuk azt a sebesség kétszeresével
        // Ha így még mindig érintkezne ugyanazzal a brick-el, akkor visszamozgatjuk az eredeti pozícióba és a másik sebességet változtatjuk, az előzőt pedig
        // visszaállítjuk az eredetire. Amelyik esetben nincs érintkezés, az a jó változtatás.
        // A '2' itt most egy kicsit magic number, egyszeres sebességgel valamivel több hiba csúszott be tesztelés közben. (Pl mindkét esetben érintkezett a brick-el.)
        ShapeBall.velocityX = -ShapeBall.velocityX;
        ShapeBall.move(2 * ShapeBall.velocityX, 2 * ShapeBall.velocityY);
        if(isIntersecting(ShapeBrick, ShapeBall))
        {
                ShapeBall.move(-2 * ShapeBall.velocityX, -2 * ShapeBall.velocityY);

                ShapeBall.velocityX = -ShapeBall.velocityX;
                ShapeBall.velocityY = -ShapeBall.velocityY;
        } else
        {
                ShapeBall.move(-2 * ShapeBall.velocityX, -2 * ShapeBall.velocityY);
        }
}

//Function for clear screen
void clearScreen() { gout << move_to(0,0) << color(0,0,0) << box(windowWidth, windowHeight); }



  ///***********************************************************************\\\
 ///**************************GAMEPLAY CLASSES***************************\\\
///**************************************************************************\\\

// Class of the _Game_
struct Game
{
private:
        bool isStarted{false};
        bool isRunning{true};
        Paddle paddle;
        Ball ball;
        vector<Brick> bricks;
public:
        Game()
        {
                paddle = Paddle{windowWidth / 2, windowHeight - 50};
                ball = Ball{paddle};

                //Create the bricks in the vector
                for(int i{1}; i <= brickCountX; i++)
                    for(int j{1}; j <= brickCountY; j++)
                        bricks.emplace_back(i * ((brickWidth / 2) + 35), j * ((brickHeight / 2) + 15));
        }

        //Function for running a game
        void run()
        {
                event game_ev;
                gin.timer(5);

                //Main loop of the game
                while(gin >> game_ev && isRunning)
                {
                        if(ball.isLost)    //testing if we can't hit back the ball (game over)
                        {
                                game_over();
                                break;
                        }

                        //Drawing phase when the timer ticks (and update the ball)
                        if(game_ev.type == ev_timer)
                        {
                                clearScreen();
                                ball.draw();
                                paddle.draw();
                                for(auto brick : bricks)
                                {
                                        if(!brick.isDestroyed) brick.draw();
                                }
                                if(isStarted) ball.update();
                                else
                                {
                                        ball.update_beforeStarted(paddle);
                                        gout << move_to((windowWidth / 2) -100, windowHeight / 2) << color(255, 255, 255) << text("Press _left mouse button_ to start the ball.");
                                }

                                gout << refresh;
                        }

                        //Input phase
                        int notDestroyedBricks{0};
                        if(game_ev.type == ev_mouse && game_ev.button == btn_left) isStarted = true;
                        if(game_ev.type == ev_mouse) paddle.update(game_ev.pos_x);
                        if(game_ev.type == ev_key && game_ev.keycode == key_escape) break;

                        //Collision testing phase
                        testCollision(paddle, ball);
                        for(auto& brick : bricks)
                        {
                                if(brick.isDestroyed) continue;
                                testCollision(brick, ball);
                                notDestroyedBricks++;
                                if(brick.isDestroyed) break;
                        }

                        //Stop running when destroyed all bricks
                        if(notDestroyedBricks == 0) isRunning = false;

                }
        }

        //Function for handle 'game over' state
        void game_over()
        {
                event go_ev;
                while(gin >> go_ev)
                {
                        clearScreen();
                        gout << move_to(windowWidth / 2, windowHeight / 2) << color(255, 255, 255) << text("GAME OVER");
                        gout << move_to(windowWidth / 2, (windowHeight / 2) + 20) << color(255, 255, 255) << text("Press enter to return to the main menu.");

                        if(go_ev.type == ev_key && (go_ev.keycode == key_enter || go_ev.keycode == key_escape)) break;

                        gout << refresh;
                }
        }
};

//Class of menu buttons
struct Button
{
private:
        int pos_x, pos_y;
        string button_text{"newbutton"};
        bool isActive{false};
public:
        Button(int x, int y, string text)
        {
                pos_x = x;
                pos_y = y;
                button_text = text;
        }
        void activate() { isActive = true; }
        void deactivate() { isActive = false; }
        bool isActivated() const { return isActive; }
        void draw() const
        {
                if(!isActive) gout << move_to(pos_x, pos_y) << color(255, 255, 255) << text(button_text);
                    else gout << move_to(pos_x, pos_y) << color(255, 0, 0) << text(button_text);
        }
};

//Class of main menu
struct Menu
{
private:
        Button *play, *exit;
public:
        Menu()
        {
                play = new Button(windowWidth / 2, windowHeight / 2, "PLAY");
                exit = new Button(windowWidth / 2, (windowHeight / 2) + 20, "EXIT");
        }
        ~Menu()
        {
                delete play;
                delete exit;
        }
        void open()
        {
                bool isMenuRunning{true};

                play->activate();

                event menu_ev;
                gin.timer(1);

                //Main loop of the main menu (and also the main loop of the whole program)
                while(gin >> menu_ev && isMenuRunning)
                {
                        clearScreen();

                        //Input phase
                        if(menu_ev.type == ev_key && menu_ev.keycode == key_down) { exit->activate(); play->deactivate(); }
                        if(menu_ev.type == ev_key && menu_ev.keycode == key_up) { exit->deactivate(); play->activate(); }

                        if(menu_ev.type == ev_key && menu_ev.keycode == key_enter)
                        {
                                if(play->isActivated()) Game{}.run();
                                    else isMenuRunning = false;
                        }

                        if(menu_ev.type == ev_key && menu_ev.keycode == key_escape) isMenuRunning = false;

                        //Drawing phase
                        gout << move_to((windowWidth / 2) - 60, 40) << color(255, 255, 255) << text("BRICK DESTROYER GAME");
                        gout << move_to(0, windowHeight - 40) << color(255, 255, 255) << text("Note: You can handle main menu with 'up' and 'down' arrows and key 'enter', and the game with mouse.");

                        play->draw();
                        exit->draw();

                        gout << refresh;
                }
        }
};


///**********MAIN FUNCTION**********\\\

int main()
{
        gout.open(windowWidth, windowHeight);

        Menu{}.open();

        return 0;
}
