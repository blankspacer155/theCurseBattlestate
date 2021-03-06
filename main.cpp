#include <vector>
#include <stack>
#include <map>
#include <iostream>
#include <sstream>
#include <functional>


#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include "SFML/Window.hpp"
#include "SFML/Audio.hpp"
#include "SFML/Network.hpp"

using namespace std;
using namespace sf;

class Status
{
public:

	//variables
	bool isdead = false;
	int hp = 600;
	int max_hp = 600;
	int att =150;
	int def = 10;
	int stamina = 100;
	int max_stamina = 100;
	int lvl = 1;
	int current_exp = 0;
	int max_exp = 500;

	int damageReceived;

	//only enemy
	int droprate = 100;
	int dropnum = 1;
	bool isAttacked = false;

	//item
	int itemnum[4] = { 0 };
	int atkchange= 0,defchange=0;
	//skill
	int skillcost[2] = {20,50};
	
	//function
	
	void UpdateHpbar(const float dt)
	{
	}
	void ItemDrop(Status* recieve, int droprate, int dropnum)
	{
		int chance = rand() % 100 + 1;
		if (chance <= droprate)
		{
			for (int i = 0; i < dropnum; i++)
			{
				int ind = rand() % 4;
				recieve->itemnum[ind]++;
			}
		}
	}
	void isDie()
	{
		if (this->hp <= 0)
		{
			this->isdead = true;
		}
	}
	void UpdatePlayerlevel()
	{
		while (this->current_exp>=this->max_exp) {
			this->current_exp -= this->max_exp;
			this->lvl++;
			max_exp = max_exp + 30 * (this->lvl - 1);
			this->hp = this->max_hp + 30 * (this->lvl - 1);
			this->max_hp = this->hp;
			this->stamina = this->max_stamina + 20 * (this->lvl - 1);
			this->max_stamina = this->stamina;
			this->att +=  10 * (this->lvl - 1);
			this->def += 5 * (this->lvl - 1);
			cout << "lvl up!!! " << this->lvl;
		}
				
	}


	void InitEnemystatus(int stagelevel)
	{
		this->hp = 200 * stagelevel;
		this->max_hp = 200 * stagelevel;
		this->att = 30 * stagelevel;
		this->def = 10 * stagelevel;
		this->droprate = 100;
		this->dropnum = 1;
	}

	void skillAttack(Status* target, float multiplier)
	{
		int tempatt = this->att;
		this->att = this -> att * multiplier;
		this->attacking(target);
		this->att = tempatt;
	}

	void attacking(Status* target) {
		int critrate = rand() % 100 + 1;
		int damage;

		if (critrate <= 10) damage = this->att * 2 - target->def;  //crit
		else damage = this->att - target->def;              //normal

		if (damage <= 0) damage = 0;   //def>atk

		target->hp -= damage;
		target->damageReceived = damage;
	}

};



class HitboxComponent
{
public:
	//Variables
	Sprite& sprite;
	RectangleShape hitbox;
	float offsetX;
	float offsetY;

	//Constructor / Destructor
	HitboxComponent(Sprite& sprite, float offset_x, float offset_y, float width, float height)
		:sprite(sprite), offsetX(offset_x), offsetY(offset_y)
	{
		this->hitbox.setPosition(this->sprite.getPosition().x + offset_x, this->sprite.getPosition().y + offset_y);
		this->hitbox.setSize(Vector2f(width, height));
		this->hitbox.setFillColor(Color::Transparent);
		this->hitbox.setOutlineThickness(1.f);
		this->hitbox.setOutlineColor(Color::Black);
	}

	virtual ~HitboxComponent()
	{

	}

	//Accessors
	const FloatRect& getGlobalBounds() const
	{
		return this->hitbox.getGlobalBounds();
	}

	//Functions
	bool checkIntersect(const FloatRect& frect)
	{
		return this->hitbox.getGlobalBounds().intersects(frect);
	}

	void update()
	{
		this->hitbox.setPosition(this->sprite.getPosition().x + this->offsetX, this->sprite.getPosition().y + this->offsetY);
	}

	void render(RenderTarget& target)
	{
		//target.draw(this->hitbox);
	}
};

enum class movementState { IDLE = 0, MOVING_UP, MOVING_LEFT, MOVING_DOWN, MOVING_RIGHT };

class MovementComponent
{
public:
	//Variables
	Sprite& sprite;
	Vector2f velocity;

	float maxVelocity;
	movementState state;

	//Initilizer functions

	//Constructor / Destructor
	MovementComponent(Sprite& sprite, float maxVelocity)
		: sprite(sprite), velocity(Vector2f(0, 0)), maxVelocity(maxVelocity), state(movementState::IDLE)
	{

	}

	virtual ~MovementComponent()
	{

	}

	//Accessors
	const Vector2f& getVelocity() const
	{
		return this->velocity;
	}

	const int getState() const
	{
		if (this->velocity.x == 0.f && this->velocity.y == 0.f)
			switch (state)
			{
			case movementState::MOVING_UP:
				return 1;
				break;
			case movementState::MOVING_LEFT:
				return 2;
				break;
			case movementState::MOVING_DOWN:
				return 3;
				break;
			case movementState::MOVING_RIGHT:
				return 4;
				break;
			default:
				break;
			}
		else if (this->velocity.y < 0)
			return 5;
		else if (this->velocity.x < 0)
			return 6;
		else if (this->velocity.y > 0)
			return 7;
		else if (this->velocity.x > 0)
			return 8;

		return 0;
	}

	//Functions
	void update()
	{
		this->sprite.move(velocity);
		if (this->velocity.y < 0)
			this->state = movementState::MOVING_UP;
		else if (this->velocity.x < 0)
			this->state = movementState::MOVING_LEFT;
		else if (this->velocity.y > 0)
			this->state = movementState::MOVING_DOWN;
		else if (this->velocity.x > 0)
			this->state = movementState::MOVING_RIGHT;
	}

	void move(const float dir_x, const float dir_y, const float& dt)
	{
		this->velocity = Vector2f(dir_x * maxVelocity * dt, dir_y * maxVelocity * dt);
	}
};

class AnimationComponent
{
public:
	class Animation
	{
	public:
		//Variables
		Sprite& sprite;
		Texture& textureSheet;
		float animationTimer;
		float timer;
		int width;
		int height;
		bool end;
		vector<IntRect> rect;

		Animation(Sprite& sprite, Texture& textureSheet, float animation_timer, int start_frame_x, int start_frame_y, int frames_x, int frames_y, int width, int height)
			: sprite(sprite), textureSheet(textureSheet), animationTimer(animation_timer / (frames_x + 1 - start_frame_x)), width(width), height(height)
		{
			this->end = false;
			this->timer = 0.f;
			for (int i = 0; i < frames_x - start_frame_x + 1; i++)
				rect.push_back(IntRect(start_frame_x * width + i * width, start_frame_y * height, width, height));
			this->sprite.setTexture(this->textureSheet);
			this->sprite.setTextureRect(IntRect(start_frame_x * width, start_frame_y * height, width, height));
		}

		//Functions
		void play(const float& dt)
		{
			//Update timer
			this->timer += dt / this->animationTimer;
			size_t n = rect.size();
			if (this->timer >= n) {
				this->timer -= n; this->end = 1;
			}
			if (n > 0) sprite.setTextureRect(rect[int(this->timer)]);
		}

		//Accessors
		bool isEnd()
		{
			return this->end;
		}

		void reset()
		{
			this->timer = 0.f;
		}
	};
	//Variables
	map<string, Animation* > animations;
	Sprite& sprite;
	Texture& textureSheet;

	//Constructor / Destructor
	AnimationComponent(Sprite& sprite, Texture& texture_sheet)
		:sprite(sprite), textureSheet(texture_sheet)
	{

	}

	virtual ~AnimationComponent()
	{
		for (auto& i : this->animations)
		{
			delete i.second;
		}
	}

	//Functions
	void addAnimation(string key, float animation_timer, int start_frame_x, int start_frame_y, int frames_x, int frames_y, int width, int height)
	{
		this->animations[key] = new Animation(this->sprite, this->textureSheet, animation_timer, start_frame_x, start_frame_y, frames_x, frames_y, width, height);
	}

	void deleteAnimation(const string key)
	{
		this->animations.erase(key);
	}

	void play(const string key, const float& dt)
	{
		this->animations[key]->play(dt);
	}

	bool isEnd(const string key)
	{
		return this->animations[key]->isEnd();
	}

	void reset()
	{
		for (auto& i : this->animations)
		{
			i.second->reset();
		}
	}
};

class Entity
{
public:
	//Variables
	Sprite sprite;
	Texture texture;

	HitboxComponent* hitboxComponent;
	MovementComponent* movementComponent;
	AnimationComponent* animationComponent;

	bool isPressthenRelease[2];

	//Initilizer functions
	virtual void initVariables()
	{
		this->hitboxComponent = NULL;
		this->movementComponent = NULL;
		this->isPressthenRelease[0] = false;
		this->isPressthenRelease[1] = false;
	}

	//Constructor / Destructor
	Entity()
	{
		this->initVariables();
		this->createHitboxComponent(this->sprite, 0.f, 0.f, 45.f, 105.f);
		this->createMovementComponent(300.f);
		this->createAnimationComponent(this->texture);
	}

	virtual ~Entity()
	{
		delete this->hitboxComponent;
		delete this->movementComponent;
		delete this->animationComponent;
	}

	//Accessors
	const FloatRect& getGlobalBounds() const
	{
		return this->hitboxComponent->getGlobalBounds();
	}

	const Vector2f& getPosition() const
	{
		return this->sprite.getPosition();
	}

	const bool IsMousePress()
	{
		if (Mouse::isButtonPressed(Mouse::Left))
		{
			return true;
		}
		else return false;
	}

	const bool IsMouseRelease()
	{
		if (!(Mouse::isButtonPressed(Mouse::Left)))
		{
			return true;
		}
		else return false;
	}

	const bool isClickat(const Vector2f mousePos)
	{
		if (this->sprite.getGlobalBounds().contains(mousePos)) {
			if (this->IsMousePress())
			{
				isPressthenRelease[0] = true;
				isPressthenRelease[1] = false;
			}
			if (this->IsMouseRelease())isPressthenRelease[1] = true;

			if (isPressthenRelease[0] and isPressthenRelease[1])
			{

				isPressthenRelease[0] = false;
				isPressthenRelease[1] = false;

				return true;
			}
		}
		return false;
	}

	//Component functions

	void setTexture(Texture& texture)
	{
		this->texture = texture;
		this->sprite.setTexture(texture);
	}

	void createHitboxComponent(Sprite& sprite, float offset_x, float offset_y, float width, float height)
	{
		this->hitboxComponent = new HitboxComponent(sprite, offset_x, offset_y, width, height);
	}

	void createMovementComponent(const float velocity)
	{
		this->movementComponent = new MovementComponent(this->sprite, velocity);
	}

	void createAnimationComponent(Texture& texture_sheet)
	{
		this->animationComponent = new AnimationComponent(this->sprite, texture_sheet);
	}

	//Functions
	void setColor(Color color)
	{
		this->sprite.setColor(color);
	}
	void setCursorPosition(const Entity& target)
	{
		this->sprite.setPosition(
			target.getPosition().x + target.getGlobalBounds().width / 2.f,
			target.getPosition().y - target.getGlobalBounds().height);
	}

	virtual void setScale(const float x, const float y)
	{
		this->sprite.setScale(x, y);
	}

	virtual void setOrigin(const float x, const float y)
	{
		this->sprite.setOrigin(x, y);
	}

	virtual void setPosition(const float x, const float y)
	{
		this->sprite.setPosition(x, y);
	}

	virtual void move(const float dir_x, const float dir_y, const float& dt)
	{
		if (this->movementComponent)
		{
			this->movementComponent->move(dir_x, dir_y, dt); //Sets velocity
		}
	}

	virtual void update(const float& dt)
	{

	}

	virtual void render(RenderTarget& target)
	{
		target.draw(this->sprite);
	}
};

class ShowText
{
public:
	Text text;
	float textshowtime=0.5;
	float textclock=0;
	bool isActive = false;
	bool isTextshowend=true;

	
	//function

	void AlwaysUpdatetext(string firstword,int stat,const float dt)
	{
		this->textclock += dt;
		
		string statstr=this->intTostring(stat);
		this->text.setString(firstword + statstr);

		if (this->textclock >= this->textshowtime)
		{
			this->textclock = 0;
		}
	}

	string intTostring(int num)
	{
		if (num == 0)return"0";
		int count=0;
		string text = "";
		while (num>0)
		{   
			
			text += '0' + (num % 10 );
			num /= 10;

			count++;
		}
		for (int i = 0; i < count/2; i++)
		{
			char temp;
			temp=text[i];
			int ind = count - 1 - i;
			text[i] = text[ind];
			text[ind] = temp;
		}
		return text;
	}
	
	void setText(string word)
	{
		this->text.setString(word);

	}
	
	void setBase(Entity* targetentity, unsigned int size, Font* font, Color color, float x, float y)
	{
		
			
			this->text.setFont(*font);
			this->text.setFillColor(color);
			this->text.setCharacterSize(size);
			if (targetentity == NULL)this->text.setPosition(Vector2f(x, y));
			else
			{
				this->text.setPosition(targetentity->getPosition().x + x, targetentity->getPosition().y + y);
			}
		

	}
	
	void update(const float dt)
	{	this->textclock += dt;
		if (this->isActive)
		{
			
			if (this->textclock>=this->textshowtime)
			{
				this->textclock = 0;
				this->isActive = false;
				this->isTextshowend = true;
				this->setText("");
			}
		}
		else
		{
			this->textclock = 0;
			this->setText("");
		}

	}

	//construct des
	ShowText()
	{

	}
	~ShowText()
	{

	}


};
class Player : public Entity
{
public:
	//Variables
	Sprite light;
	Texture lightTexture;

	//Initilizer functions
	void initVariables()
	{

	}

	void initComponents()
	{

	}

	//Constructor / Destructor
	Player(float x, float y, Texture& texture_sheet)
	{
		this->initVariables();

		this->setOrigin(40.f, 10.f);
		this->setPosition(x, y);

		this->createHitboxComponent(this->sprite, 0.f, 0.f, 45.f, 105.f);
		this->createMovementComponent(300.f);
		this->createAnimationComponent(texture_sheet);

		this->lightTexture.loadFromFile("Images/light.png");
		this->light.setTexture(lightTexture);
		this->light.setOrigin(this->light.getGlobalBounds().width / 2, this->light.getGlobalBounds().height / 2);
		this->light.setColor(Color::Transparent);

		this->animationComponent->addAnimation("IDLE", 0.5f, 0, 0, 0, 0, 125, 125);
		this->animationComponent->addAnimation("IDLE_UP", 0.5f, 1, 1, 1, 1, 125, 125);
		this->animationComponent->addAnimation("IDLE_LEFT", 0.5f, 1, 2, 1, 2, 125, 125);
		this->animationComponent->addAnimation("IDLE_DOWN", 0.5f, 1, 3, 1, 3, 125, 125);
		this->animationComponent->addAnimation("IDLE_RIGHT", 0.5f, 1, 4, 1, 4, 125, 125);
		this->animationComponent->addAnimation("UP", 0.5f, 0, 1, 3, 1, 125, 125);
		this->animationComponent->addAnimation("LEFT", 0.5f, 0, 2, 3, 2, 125, 125);
		this->animationComponent->addAnimation("DOWN", 0.5f, 0, 3, 3, 3, 125, 125);
		this->animationComponent->addAnimation("RIGHT", 0.5f, 0, 4, 3, 4, 125, 125);
	}

	virtual ~Player()
	{

	}

	//Component functions
	void setLight(bool x)
	{
		if (x)
			this->light.setColor(Color::Black);
		else
			this->light.setColor(Color::Transparent);
	}

	virtual void update(const float& dt)
	{
		this->movementComponent->update();

		switch (this->movementComponent->getState())
		{
		case 1:
			this->animationComponent->play("IDLE_UP", dt);
			this->animationComponent->reset();
			break;
		case 2:
			this->animationComponent->play("IDLE_LEFT", dt);
			this->animationComponent->reset();
			break;
		case 3:
			this->animationComponent->play("IDLE_DOWN", dt);
			this->animationComponent->reset();
			break;
		case 4:
			this->animationComponent->play("IDLE_RIGHT", dt);
			this->animationComponent->reset();
			break;
		case 5:
			this->animationComponent->play("UP", dt);
			break;
		case 6:
			this->animationComponent->play("LEFT", dt);
			break;
		case 7:
			this->animationComponent->play("DOWN", dt);
			break;
		case 8:
			this->animationComponent->play("RIGHT", dt);
			break;
		default:
			this->animationComponent->play("IDLE", dt);
			break;
		}

		this->hitboxComponent->update();
		this->light.setPosition(this->sprite.getPosition().x + this->getGlobalBounds().width / 2, this->sprite.getPosition().y + this->getGlobalBounds().height / 2);
	}


	virtual void render(RenderTarget& target)
	{
		target.draw(this->sprite);
		target.draw(this->light);
	}
};

class CollosionBox
{
public:
	//Variables
	RectangleShape shape;
	RectangleShape& playerShape;
	Player& player;
	RectangleShape up;
	RectangleShape left;
	RectangleShape down;
	RectangleShape right;

	//Constructor / Destructor
	CollosionBox(Player& player, float x, float y, float width, float height)
		:player(player), playerShape(player.hitboxComponent->hitbox)
	{
		this->shape.setPosition(x, y);
		this->shape.setSize(Vector2f(width, height));
		this->shape.setFillColor(Color::Black);
		this->shape.setOutlineThickness(1.f);
		this->shape.setOutlineColor(Color::Black);
		this->up.setFillColor(Color::Red);
		this->up.setPosition(x, y);
		this->up.setSize(Vector2f(width, 1.f));
		this->left.setFillColor(Color::Blue);
		this->left.setPosition(x, y);
		this->left.setSize(Vector2f(1.f, height));
		this->down.setFillColor(Color::Green);
		this->down.setPosition(x, y + height);
		this->down.setSize(Vector2f(width, 1.f));
		this->right.setFillColor(Color::Magenta);
		this->right.setPosition(x + width, y);
		this->right.setSize(Vector2f(1.f, height));
	}

	virtual ~CollosionBox()
	{

	}

	//Functions
	void update()
	{
		if (this->playerShape.getGlobalBounds().intersects(this->up.getGlobalBounds()) && this->player.movementComponent->state == movementState::MOVING_DOWN)
		{
			this->player.setPosition(this->player.getPosition().x, this->shape.getGlobalBounds().top - this->playerShape.getGlobalBounds().height);
		}
		else if (this->playerShape.getGlobalBounds().intersects(this->left.getGlobalBounds()) && this->player.movementComponent->state == movementState::MOVING_RIGHT)
		{
			this->player.setPosition(this->shape.getGlobalBounds().left - this->playerShape.getGlobalBounds().width, this->player.getPosition().y);
		}
		else if (this->playerShape.getGlobalBounds().intersects(this->down.getGlobalBounds()) && this->player.movementComponent->state == movementState::MOVING_UP)
		{
			this->player.setPosition(this->player.getPosition().x, this->shape.getGlobalBounds().top + this->shape.getGlobalBounds().height);
		}
		else if (this->playerShape.getGlobalBounds().intersects(this->right.getGlobalBounds()) && this->player.movementComponent->state == movementState::MOVING_LEFT)
		{
			this->player.setPosition(this->shape.getGlobalBounds().left + this->shape.getGlobalBounds().width, this->player.getPosition().y);
		}
	}

	void render(RenderTarget& target)
	{
		target.draw(this->shape);
	}
};

class State
{
public:
	//Variables
	RenderWindow* window;
	vector<State*>* states;
	map<string, int>* supportedKeys;
	map<string, int> keybinds;
	Font font;
	bool quit;

	Vector2i mousePosScreen;
	Vector2i mousePosWindow;
	Vector2f mousePosView;

	//Resources
	map<string, Texture> textures;

	void initFonts()
	{
		if (!this->font.loadFromFile("Fonts/2005_iannnnnJPG.ttf"))
			throw("ERROR::MAINMENUSTATE::COULD NOT LOAD FONT");
	}

	//Constructor / Destructor
	State(RenderWindow* window, vector<State*>* states)
	{
		this->window = window;
		this->supportedKeys = supportedKeys;
		this->states = states;
		this->quit = false;
	}

	virtual ~State()
	{

	}

	//Accessors
	const bool& getQuit() const
	{
		return this->quit;
	}

	//Functions
	virtual void endStateUpdate()
	{

	}

	void endState()
	{
		this->quit = true;
	}

	virtual void updateMousePositions()
	{
		this->mousePosScreen = Mouse::getPosition();
		this->mousePosWindow = Mouse::getPosition(*this->window);
		this->mousePosView = this->window->mapPixelToCoords(Mouse::getPosition(*this->window));
	}

	virtual void updateInput(const float& dt)
	{

	}

	virtual void update(const float& dt)
	{

	}

	virtual void render(RenderTarget* target = NULL)
	{

	}
};

namespace temp {

	Status* TempStatus = new Status;
}

enum class button_states { BTN_IDLE, BTN_HOVER, BTN_ACTIVE };

class Button
{
public:
	button_states buttonState;

	RectangleShape shape;
	Font* font;
	Text text;

	Color textIdleColor;
	Color textHoverColor;
	Color textActiveColor;

	Color idleColor;
	Color hoverColor;
	Color activeColor;

	bool isPressthenRelease[3] = { false,false,false };

	Button(float x, float y, float width, float height, Font* font, string text, unsigned character_size, Color text_idle_color, Color text_hover_color, Color text_active_color,
		Color idle_color, Color hover_color, Color active_color)
	{
		this->buttonState = button_states::BTN_IDLE;

		this->shape.setPosition(Vector2f(x, y));
		this->shape.setSize(Vector2f(width, height));
		this->shape.setFillColor(idle_color);

		this->font = font;
		this->text.setFont(*this->font);
		this->text.setString(text);
		this->text.setFillColor(text_idle_color);
		this->text.setCharacterSize(character_size);
		this->text.setPosition(
			this->shape.getPosition().x + this->shape.getGlobalBounds().width / 2.f - this->text.getGlobalBounds().width / 2.f,
			this->shape.getPosition().y + this->shape.getGlobalBounds().height / 2.f - this->text.getCharacterSize() / 2.f);

		this->textIdleColor = text_idle_color;
		this->textHoverColor = text_hover_color;
		this->textActiveColor = text_active_color;

		this->idleColor = idle_color;
		this->hoverColor = hover_color;
		this->activeColor = active_color;
	}

	virtual ~Button()
	{

	}

	//Accessors
	const bool isPressed() const
	{
		if (this->buttonState == button_states::BTN_ACTIVE)
			return true;

		return false;
	}
	//check click
	const bool IsMousePress() const
	{
		if (Mouse::isButtonPressed(Mouse::Left))
		{
			return true;
		}
		else return false;
	}

	const bool IsMouseRelease() const
	{
		if (!(Mouse::isButtonPressed(Mouse::Left)))
		{
			return true;
		}
		else return false;
	}

	//Functions
	void update(const Vector2f mousePos)
	{
		//Update the booleans for hover and pressed

		//Idle
		this->buttonState = button_states::BTN_IDLE;

		//Hover
		if (this->shape.getGlobalBounds().contains(mousePos))
		{
			this->buttonState = button_states::BTN_HOVER;


			if (this->IsMousePress())
			{
				isPressthenRelease[0] = true;
				isPressthenRelease[1] = false;
			}
			if (this->IsMouseRelease())isPressthenRelease[1] = true;

			if (isPressthenRelease[0] and isPressthenRelease[1])
			{
				this->buttonState = button_states::BTN_ACTIVE;
				isPressthenRelease[0] = false;
				isPressthenRelease[1] = false;
			}
		}
		else {
			isPressthenRelease[0] = false;
			isPressthenRelease[1] = false;
		}

		switch (this->buttonState)
		{
		case button_states::BTN_IDLE:
			this->shape.setFillColor(this->idleColor);
			this->text.setFillColor(this->textIdleColor);
			break;

		case button_states::BTN_HOVER:
			this->shape.setFillColor(this->hoverColor);
			this->text.setFillColor(this->textHoverColor);
			break;

		case button_states::BTN_ACTIVE:
			this->shape.setFillColor(this->activeColor);
			this->text.setFillColor(this->textActiveColor);
			break;

		default:
			this->shape.setFillColor(Color::Red);
			this->text.setFillColor(Color::Blue);
			break;
		}
	}

	void render(RenderTarget& target)
	{
		target.draw(this->shape);
		target.draw(this->text);
	}
};

class BattleState : public State {

public:
	//variables
	Texture bgtexture;
	RectangleShape background;
	Font font;

	//status
	Status playerstatus, * enemystatus, * ptplayerstatus;
	Status* targetstatus;
	int enemyAttacked = 0;
	int enemydead = 0;
	//target cursor
	Entity targetCursor, * targetentity;
	Texture targetCursortex;


	//button
	map<string, Button*> Mainbuttons, Itembuttons,Skillbuttons, * currentbutton;

	//entity player&ene
	Entity player;
	Entity* enemy;
	int enemynum = 2;     //enemy number
	Texture playertexture[3], enemytexture[3];  //idle and attacking
	float enemyposition[2] =  {1000.f,100.f};

	int playerstate=0;//0idle 1attack 2get attack 
	int *enemystate;//0 idle 1attack 2get attack
	//round
	int round = 0;
	int roundlimit;
	

	//stage 
	
	int stagelevel;
	bool isbossstage;

	//turn 
	bool isPlayerturn;
	bool isEnemyturn;
	bool isStageturn;
	bool isBattleend=false;
	bool isItemUsed = false;
	bool isSkillUsed = false;
	
	//item window
	RectangleShape Itemwindow;
	Texture ItemwindowTexture;
	bool isItemwindowActive;
	ShowText Itemstatus[4];

	//skill window 
	RectangleShape Skillwindow;
	Texture SkillwindowTexture;
	bool isSkillwindowActive;
	ShowText Skillstatus[2];


	//time
	double Actionclock=0;
	double Textclock = 0;
	bool isBusy = false;

	//text
	ShowText playerStatText[5], playerdamage, * enemytext, stagetext,Roundtext;
	string PlayerstatString[5] = { "HP: ","MP: ","LV.: ","ATK: ","DEF: " };
	int *playerstatusstat[5] = {&this->playerstatus.hp,&this->playerstatus.stamina,&this->playerstatus.lvl,
									&this->playerstatus.att,&this->playerstatus.def};
	


	//hp mp bar
	Sprite playerhpbar,playerstaminabar,*enemyhpbar;
	Texture hpbarTexture[11],StaminaTexture[11];
	IntRect barind[11] = { {0,0,800,100},{800,0,800,100} ,{1600,0,800,100} 
							,{0,100,800,100},{800,100,800,100} ,{1600,100,800,100} 
							,{0,200,800,100},{800,200,800,100} ,{1600,200,800,100}
							,{0,300,800,100},{800,300,800,100}  };
		


	//iniilizer function
	void initBarTexture()
	{
		for (int i = 0; i < 11; i++)
		{
			if (!this->hpbarTexture[i].loadFromFile("Images/Sprites/HP/Pac HP.png", this->barind[i]))
			{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
			}
			if (!this->StaminaTexture[i].loadFromFile("Images/Sprites/SP/SP PAC.png", this->barind[i]))
			{
				throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
			}
		}
		
	}
	void createBar(Entity *targetentity,Sprite *targetbar,float x, float y, float scalex, float scaley,char type)
	{
		if (type == 'H') {
			targetbar->setTexture(this->hpbarTexture[0]);
		}
		//S for Stamina
		else
		{
			targetbar->setTexture(this->StaminaTexture[0]);
		}
		targetbar->setScale(scalex, scaley);
		if (!targetentity == NULL)targetbar->setPosition(targetentity->getPosition().x + x, targetentity->getPosition().y + y);
		else targetbar->setPosition(x,y);
	}
	void initItemwindow()
	{
		if (!this->ItemwindowTexture.loadFromFile("Images/Itemwindow.jpg"))
		{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
		}

		this->Itemwindow.setSize(Vector2f(300.,600.));
		this->Itemwindow.setTexture(&this->ItemwindowTexture);
		this->Itemwindow.setPosition(Vector2f(700.f, 100.f));

		this->isItemwindowActive = false;
		//set item remain
		this->Itemstatus[0].setBase(NULL,30,&this->font,Color::White,900,100);
		this->Itemstatus[1].setBase(NULL, 30, &this->font, Color::White, 900, 200);
		this->Itemstatus[2].setBase(NULL, 30, &this->font, Color::White, 900, 300);
		this->Itemstatus[3].setBase(NULL, 30, &this->font, Color::White, 900,400);
	}
	void initSkillwindow()
	{
		if (!this->SkillwindowTexture.loadFromFile("Images/Skillwindow.png"))
		{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
		}
		
		this->Skillwindow.setSize(Vector2f(300., 600.));
		this->Skillwindow.setTexture(&this->SkillwindowTexture);
		this->Skillwindow.setPosition(Vector2f(700.f, 100.f));
		//set skill mp use
		this->Skillstatus[0].setBase(NULL, 30, &this->font, Color::White,700, 300);
		this->Skillstatus[1].setBase(NULL, 30, &this->font, Color::White, 700, 400);

		this->isSkillwindowActive = false;
	}
	void initPlayerEnemyTexture()
	{
		int playery = 125,enemyx=0;
		for (int i = 0; i < 3; i++)
		{
			if (!this->playertexture[i].loadFromFile("Images/Sprites/Player/test.png",IntRect(0,playery,125,125)))
			{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
			}
			playery += 125;
			if (!this->enemytexture[i].loadFromFile("Images/monster_on_map/map1/testenemy.png",IntRect(enemyx,0,120,120)))
			{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
			}
			enemyx += 120;
		}
	}
	void initPlayer()
	{
		//turn
		this->isPlayerturn = true;
		this->isEnemyturn = false;
		this->isStageturn = false;

		//load player status

		this->playerstatus = *temp::TempStatus;
		ptplayerstatus = &playerstatus;
		//set position
		this->player.setPosition(400.f, 400.f);
		this->player.setTexture(this->playertexture[0]);

		this->playerstate = 0;//set idle

		this->playerdamage.setBase(&this->player,50,&this->font,Color::White,-10,-10);
		this->playerdamage.setText("0");
		this->playerdamage.textshowtime = 1;
		//set player stat text
		this->playerStatText[0].setBase(NULL,40,&this->font,Color::Black,1120,830);
		this->playerStatText[1].setBase(NULL, 40, &this->font, Color::Black, 1120, 880);
		this->playerStatText[2].setBase(NULL, 40, &this->font, Color::Black, 1500, 830);
		this->playerStatText[3].setBase(NULL, 40, &this->font, Color::Black, 1500, 880);
		this->playerStatText[4].setBase(NULL, 40, &this->font, Color::Black, 1500, 930);
		//hp bar Stamina bar
		this->createBar(NULL,&this->playerhpbar,1100,860,0.5,0.5,'H');
		this->createBar(NULL, &this->playerstaminabar, 1000, 910, 0.5, 0.5, 'S');

	}

	void initEnemy()
	{
		this->enemystatus = new Status[enemynum];
		this->enemy = new Entity[enemynum];
		this->enemytext = new ShowText[enemynum];
		this->enemyhpbar = new Sprite[enemynum];
		this->enemystate = new int[enemynum];
		this->addEnemy();
	}
	void deleteEnemy()
	{
		
		delete[] this->enemy;
		delete[] this->enemystatus;
		delete[] this->enemytext;
		delete[] this->enemyhpbar;
		delete[] this->enemystate;
		this->enemystatus = new Status[enemynum];
		this->enemy = new Entity[enemynum];
		this->enemytext = new ShowText[enemynum];
		this->enemyhpbar = new Sprite[enemynum];
		this->enemystate = new int[enemynum];

		
	}

	void addEnemy()
	{
		this->enemyAttacked = 0;
		
		float tempposition=enemyposition[1];
		for (int i = 0; i < enemynum; i++)
		{
			this->enemy[i].setPosition(this->enemyposition[0], tempposition+=200.f);
			this->enemy[i].setTexture(this->enemytexture[0]);
			//set state
			this->enemystate[i] = 0; ///idle

			//init enemystatus
			this->enemystatus[i].InitEnemystatus(this->stagelevel);
			//init enemy text
			this->enemytext[i].setBase(&this->enemy[i],  50, &this->font, Color::White, -50, -10);
			this->enemytext[i].setText("99");
			//set enemy hp bar
			this->createBar(&this->enemy[i],&this->enemyhpbar[i],25,-10,0.25,0.25,'H');

		}

		//set default target
		this->targetstatus = &enemystatus[0];
		this->targetentity = &enemy[0];
		//target curser
		if (!this->targetCursortex.loadFromFile("Images/targetcursor.png"))
		{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
		}
		this->targetCursor.setTexture(this->targetCursortex);
		this->targetCursor.setCursorPosition(*this->targetentity);

	}
	
	void initStagetext()
	{
		this->stagetext.setBase(NULL, 200, &this->font, Color::White, 600, 400);
		this->stagetext.textshowtime = 1;
		this->Roundtext.setBase(NULL, 100, &this->font, Color::White, 600, 50);

	}
	void initVariables(int round, int lvl, bool isboss)
	{
		this->roundlimit = round;
		this->stagelevel = lvl;
		this->isbossstage = isboss;
	}
	void initBackground()
	{

		if (!this->bgtexture.loadFromFile("Images/battle_scene/map1.png"))
		{
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
		}

		this->background.setSize(Vector2f(this->window->getView().getSize()));
		this->background.setTexture(&this->bgtexture);

	}
	void initFonts()
	{
		if (!this->font.loadFromFile("Fonts/2005_iannnnnJPG.ttf"))
		{
			throw("ERROR::MAINMENUSTATE::COULD NOT LOAD FONT");
		}
	}

	void initButtons()
	{
		this->Mainbuttons["ATTACK"] = new Button(50, 850,150,100, &this->font, "Attack",50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Mainbuttons["ITEM"] = new Button(300, 850, 150, 100, &this->font, "ITEM", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Mainbuttons["SKILL"] = new Button(550, 850, 150, 100, &this->font, "SKILL", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Mainbuttons["RUN"] = new Button(800, 850, 150, 100, &this->font, "RUN", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));

		this->Itembuttons["ITEM1"] = new Button(1000, 100, 150, 100, &this->font, "item1", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Itembuttons["ITEM2"] = new Button(1000, 200, 150, 100, &this->font, "ITEM2", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Itembuttons["ITEM3"] = new Button(1000, 300, 150, 100, &this->font, "item3", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Itembuttons["ITEM4"] = new Button(1000, 400, 150, 100, &this->font, "item4",50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Itembuttons["EXITITEM"] = new Button(1000, 500, 150, 100, &this->font, "exit", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));

		this->Skillbuttons["SKILL1"] = new Button(800, 300, 150, 100, &this->font, "skill1", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Skillbuttons["SKILL2"] = new Button(800, 400, 150, 100, &this->font, "skill2", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
		this->Skillbuttons["EXITSKILL"] = new Button(800, 500, 150, 100, &this->font, "exit", 50, Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));

		//init curerent button
		currentbutton = &this->Mainbuttons;
	}
	void deleteButtons()
	{
		for (auto it = this->Mainbuttons.begin(); it != this->Mainbuttons.end(); ++it)
		{
			delete it->second;
		}
		for (auto it = this->Itembuttons.begin(); it != this->Itembuttons.end(); ++it)
		{
			delete it->second;
		}
		for (auto it = this->Skillbuttons.begin(); it != this->Skillbuttons.end(); ++it)
		{
			delete it->second;
		}
	}

	//Constructor / Destructor
	BattleState(RenderWindow* window, vector<State*>* states, int Maxround, int Stagelevel, bool Isbossstage)
		: State(window, states)
	{
		this->initVariables(Maxround, Stagelevel, Isbossstage);
		this->initBarTexture();
		this->initPlayerEnemyTexture();
		this->initPlayer();
		this->initEnemy();
		this->initBackground();
		this->initFonts();
		this->initButtons();
		this->initItemwindow();
		this->initSkillwindow();
		this->initStagetext();
		
	}

	virtual ~BattleState()
	{
		this->deleteButtons();
		delete[] this->enemy;
		delete[] this->enemystatus;
		delete[] this->enemytext;
		delete[] this->enemyhpbar;
		delete[] this->enemystate;
	}


	//Functions
	void UpdateStatus()
	{	
		for (int ind = 0; ind < enemynum; ind++)
		{
			if (!enemystatus[ind].isdead)enemystatus[ind].isDie();
			else continue;

			if (enemystatus[ind].isdead)
			{	//check enemy remain
				this->enemydead++;

				//update Item drop
				this->enemystatus[ind].ItemDrop(&this->playerstatus, enemystatus[ind].droprate, enemystatus[ind].dropnum);
				cout << "Item now " << this->playerstatus.itemnum[0] << this->playerstatus.itemnum[1]
					<< this->playerstatus.itemnum[2] << this->playerstatus.itemnum[3] << endl;

				//set auto next target

				for (int i = 0; i < enemynum; i++)
				{
					if (this->enemystatus[i].isdead == false)
					{

						targetstatus = &enemystatus[i];
						targetentity = &enemy[i];
						this->targetCursor.setCursorPosition(*this->targetentity);
						break;
					}
				}
				//update player status

				this->playerstatus.current_exp += 100;

				//check if player level up
				if (this->playerstatus.current_exp >= this->playerstatus.max_exp)
				{
					this->playerstatus.UpdatePlayerlevel();
					this->playerdamage.setText("LEVEL UP!");
					this->playerdamage.isActive = true;
					this->playerdamage.isTextshowend = false;
				}
				cout << this->playerstatus.current_exp << " " << this->playerstatus.lvl << " " << this->playerstatus.att << endl;

			}
			else if(!isPlayerturn) isEnemyturn = true;
		}
		// check if kill all
		bool isenemytextactive = false;
		for (int i = 0; i < enemynum; i++)if (enemytext[i].isActive)isenemytextactive = true;
		if (enemydead == enemynum and !isenemytextactive)
		{
			this->round++;
			this->deleteEnemy();
			this->enemydead = 0;
			this->isStageturn = true;
			//round clear


			if (this->round == this->roundlimit)
			{
				//stage clear

				this->stagetext.setText("Stage Clear");
				this->stagetext.isActive = true;
				this->stagetext.isTextshowend = false;
				this->isBattleend = true;
				//backup player data
				playerstatus.att -= playerstatus.atkchange;
				playerstatus.def -= playerstatus.defchange;
				*temp::TempStatus = playerstatus;
			}
			else
			{
				this->stagetext.setText("Round Clear");
				this->stagetext.isActive = true;
				this->stagetext.isTextshowend = false;

			}
		}
	}
	void UpdateEnemyAttack()
	{
		//if (!this->isBusy) {
			//enemy turn
			if (this->isEnemyturn)
			{
				int aviable=0;
				bool isremain = false;
				for (int i = 0; i < this->enemynum; i++)
				{
					if (!this->enemystatus[i].isdead and !this->enemystatus[i].isAttacked)
					{
						aviable = i;
						isremain = true;
						break;
					}
				}
				
					if (!(this->enemystatus[aviable].isdead)  and this->playerdamage.isTextshowend and isremain)
					{
						this->enemystatus[aviable].isAttacked = true;
						this->enemystatus[aviable].attacking(ptplayerstatus);
						//show text
						this->playerdamage.setText(this->playerdamage.intTostring(this->playerstatus.damageReceived));
						this->playerdamage.isActive = true;
						this->playerdamage.isTextshowend = false;
						//set state
						this->enemystate[aviable] = 1;
						this->playerstate = 2;
						this->isBusy = true;

						cout << "enemy " <<aviable << " attack " << "Player Hp remain " << this->playerstatus.hp << endl;

						//check player dead
						this->playerstatus.isDie();
						if (this->playerstatus.isdead)
						{
							//show stage text
							this->isStageturn = true;
							this->stagetext.setText("Player dead");
							this->stagetext.isActive = true;
							this->stagetext.isTextshowend = false;
							this->isPlayerturn = false;
							this->isBattleend = true;
							//player daed
							

						}
						this->enemyAttacked++;
					
				}
				if (!this->isStageturn and this->enemyAttacked==this->enemynum-this->enemydead)
				{
				this->isEnemyturn = false;
				 this->isPlayerturn = true;
				 this->enemyAttacked = 0;
				 
				}
				
			}
		//}
	}
	void UpdateAttack()
	{
		//Attack button only
		//Update player input  oon turn
		
			if (this->Mainbuttons["ATTACK"]->isPressed())
			{

				//attack
				auto targetindex = targetstatus - &enemystatus[0];
				this->playerstatus.attacking(targetstatus);
				cout << "enemy " << targetindex << " " << targetstatus->hp << endl;
				//show damage
				this->enemytext[targetindex].setText(this->enemytext[targetindex].intTostring(this->enemystatus[targetindex].damageReceived));
				this->enemytext[targetindex].isActive = true;
				//this->enemytext[targetindex].isTextshowend = false;

				//set state
				this->playerstate = 1;
				this->enemystate[targetindex] = 2;
				
				this->isBusy = true;
				//set enemy can attack
				//this->isEnemyturn = true;
				//change turn to enemy 
				this->isPlayerturn = false;
				
			}
			//skill zone

		
	}
	void Enemyturn()
	{
		//enemy attack
		this->UpdateEnemyAttack();
	}
	void UpdateSkillUse()
	{

		string skillname[2] = { "SKILL1","SKILL2"};
		for (int i = 0; i < 2; i++)
		{

			if (this->Skillbuttons[skillname[i]]->isPressed() and this->isPlayerturn)
			{

				if (this->playerstatus.stamina<playerstatus.skillcost[i])
				{
					cout << "Stamina for skill " << i << " not enough " << endl;
					//show text
					this->playerdamage.setText("Stamina not enough!");
					isSkillUsed = false;

				}
				else isSkillUsed = true;

				if (isSkillUsed) {
					//set target index
					auto targetindex = targetstatus - &enemystatus[0];
			
					switch (i)
					{
					case 0:
						cout << "skill 1 used" << endl;
						this->playerdamage.setText("Skill 1 Active");
						this->playerstatus.skillAttack(targetstatus,1.5);
						cout << "Skill to enemy " << targetindex << " " << targetstatus->hp << endl;
						//show damage
						this->enemytext[targetindex].setText(this->enemytext[targetindex].intTostring(this->enemystatus[targetindex].damageReceived));
						this->enemytext[targetindex].isActive = true;
						//set state
						this->playerstate = 1;
						this->enemystate[targetindex] = 2;

						playerstatus.stamina -= playerstatus.skillcost[i];
						break;
					case 1:
						cout << "skill 2 used" << endl;
						this->playerdamage.setText("Skill 2 Active");
					
						for (int i = 0; i < enemynum; i++)
						{
							if (!enemystatus[i].isdead) {
								this->playerstatus.skillAttack(&enemystatus[i], 0.5);
								cout << "Skill to enemy " << targetindex << " " << targetstatus->hp << endl;
								//show damage
								this->enemytext[i].setText(this->enemytext[i].intTostring(this->enemystatus[i].damageReceived));
								this->enemytext[i].isActive = true;
								//set state
								this->enemystate[i] = 2;
							}
						}
						this->playerstate = 1;
						playerstatus.stamina -= playerstatus.skillcost[i];
						break;
					}
				
				
				
				}
				this->playerdamage.isActive = true;
				this->playerdamage.isTextshowend = false;
			}
		}
		if (this->isSkillUsed)
		{
			//close windwo
			isSkillwindowActive = false;
			this->Skillbuttons["SKILL1"]->buttonState = button_states::BTN_IDLE;
			this->Skillbuttons["SKILL2"]->buttonState = button_states::BTN_IDLE;
			currentbutton = &this->Mainbuttons;
			//change turn
				//this->isEnemyturn = true;
				this->isPlayerturn = false;
				this->isSkillUsed = false;
			
		}
	}
	void UpdateItemUse()
	{
		string itemname[4] = { "ITEM1","ITEM2","ITEM3","ITEM4" };
		for (int i = 0; i < 4; i++)
		{
			
			if (this->Itembuttons[itemname[i]]->isPressed() and this->isPlayerturn)
			{
				
				if (this->playerstatus.itemnum[i] <= 0)
				{
					cout << "Item " << i << " not enough " << endl;
					//show text
					this->playerdamage.setText("Item "+playerdamage.intTostring(i)+" not enough!");
					isItemUsed = false;
				
				}
				else isItemUsed = true;

				if (isItemUsed) {
					float newstat;
					switch (i)
					{
					case 0:
						newstat=0.2f * this->playerstatus.att;
						this->playerstatus.att += newstat;
						this->playerdamage.setText("Atk + "+playerdamage.intTostring(newstat));
						playerstatus.atkchange += newstat;
						cout << "effect +att " << endl;
						break;
					case 1:
						newstat = 0.2f * this->playerstatus.def;
						this->playerstatus.def += newstat;
						this->playerdamage.setText("Def + " + playerdamage.intTostring(newstat));
						playerstatus.defchange += newstat;
						cout << "effect +deff" << endl;
						break;
					case 2:
						newstat=0.2f * this->playerstatus.max_hp;
						if (playerstatus.hp + newstat > playerstatus.max_hp)
						{
							newstat = playerstatus.max_hp - playerstatus.hp;
							playerstatus.hp = playerstatus.max_hp;
						}
						else playerstatus.hp += newstat;

						playerdamage.setText("HP + " + playerdamage.intTostring(newstat));
						cout << "effect +hp" << endl;
						break;
					case 3:
						newstat=0.5f * this->playerstatus.max_stamina;
						if (playerstatus.stamina + newstat > playerstatus.max_stamina)
						{
							newstat = playerstatus.max_stamina - playerstatus.stamina;
							playerstatus.stamina = playerstatus.max_stamina;
						}
						else playerstatus.stamina += newstat;
						playerdamage.setText("SP + " + playerdamage.intTostring(newstat));
						cout << "effect +stamina" << endl;
						break;

					}
					cout << "Use Item " << i << endl;
					this->playerstatus.itemnum[i]--;
					//Item disappear after use ; unlike playerstatus if dead-> status back to before battle
					temp::TempStatus->itemnum[i]--;
					cout << "Item" << i << " remain " << playerstatus.itemnum[i] << endl;
				
					
				}
					this->playerdamage.isActive = true;
					this->playerdamage.isTextshowend = false;
				
			}
		}
		if (this->isItemUsed)
		{
			if (this->isItemwindowActive==false)
			{
				this->isEnemyturn = true;
				//change turn to enemy 
				this->isPlayerturn = false;
				this->isItemUsed = false;
			}
		}
	}
	void PlayerSetTarget()
	{
		if (!this->isItemwindowActive ) {
			//set target
			if (this->enemy[0].isClickat(mousePosView) and !(this->enemystatus[0].isdead))
			{
				targetstatus = &enemystatus[0];
				targetentity = &enemy[0];
				this->targetCursor.setCursorPosition(*this->targetentity);
				cout << "Click enemy 0" << endl;
			}
			if (this->enemy[1].isClickat(mousePosView) and !(this->enemystatus[1].isdead))
			{
				targetstatus = &enemystatus[1];
				targetentity = &enemy[1];
				this->targetCursor.setCursorPosition(*this->targetentity);
				cout << "Click enemy 1" << endl;
			}if (this->enemy[2].isClickat(mousePosView) and !(this->enemystatus[2].isdead))
			{
				targetstatus = &enemystatus[2];
				targetentity = &enemy[2];
				this->targetCursor.setCursorPosition(*this->targetentity);
				cout << "Click enemy 2" << endl;
			}
		}
	}
	void ResetenemyAttack()
	{
		//reset enemy attack
		for (int i = 0; i < this->enemynum; i++)
		{
			this->enemystatus[i].isAttacked = false;
		}
		
	}

	void updateInput(const float& dt)
	{		
			//player set target by click 
			this->PlayerSetTarget();
			
			if (this->isPlayerturn and this->stagetext.isTextshowend and !this->isBusy)
			{	//reset enemy attack
				this->ResetenemyAttack();
				//update attack zone
				this->UpdateAttack();
				//update if Item use
				this->UpdateItemUse();	
				//update if skill use
				this->UpdateSkillUse();
				
			}
			//update status 
			
			if(!this->isStageturn )this->UpdateStatus();
			
			if (this->isStageturn )
			{
				
					if (this->stagetext.isTextshowend )
					{
						
							this->addEnemy();
							this->isPlayerturn = true;
							this->isStageturn = false;
							this->isBusy = false;
						
					}
					this->isEnemyturn = false;
				
				
			}
			
			 if(this->isEnemyturn and this->stagetext.isTextshowend and !this->isBusy)
			{
				this->Enemyturn();
			}
			
		
	}

	void updateButtons()
	{

		/*Updates all the buttons in the state and handles their functionlaity.*/
		for (auto& it : *currentbutton)
		{
			it.second->update(this->mousePosView);
		}

		//ITem 
		if (this->Mainbuttons["ITEM"]->isPressed())
		{
			this->isItemwindowActive = true;
			this->Mainbuttons["ITEM"]->buttonState = button_states::BTN_IDLE;
			currentbutton = &this->Itembuttons;


		}
		//close Item 
		if (this->Itembuttons["EXITITEM"]->isPressed())
		{
			this->isItemwindowActive = false;
			this->Itembuttons["EXITITEM"]->buttonState = button_states::BTN_IDLE;
			currentbutton = &this->Mainbuttons;
		}
		//skill
		if (this->Mainbuttons["SKILL"]->isPressed())
		{
			this->isSkillwindowActive = true;
			this->Mainbuttons["SKILL"]->buttonState = button_states::BTN_IDLE;
			currentbutton = &this->Skillbuttons;


		}
		//close skill 
		if (this->Skillbuttons["EXITSKILL"]->isPressed())
		{
			this->isSkillwindowActive = false;
			this->Skillbuttons["EXITSKILL"]->buttonState = button_states::BTN_IDLE;
			currentbutton = &this->Mainbuttons;
		}
		//Quit the game
		if (this->Mainbuttons["RUN"]->isPressed())
		{
			this->stagetext.setText("Running");
			this->stagetext.isActive = true;
			this->stagetext.isTextshowend = false;
			this->isBattleend = true;
		//	this->endState(); // back to game state
		}
	}

	bool isEndaction(float waittime)
	{
		if (this->Actionclock >= waittime)
		{
			this->Actionclock = 0;
			this->isBusy = false;
			return true;
		}
		else
		{
			this->isBusy = true;
			return false;
		}
	}
	
	void UpdatePlayerEnemy(const float& dt)
	{
		if (!this->isBusy)this->Actionclock = 0;
		
		this->player.setTexture(this->playertexture[0]);
		for (int i = 0; i < enemynum; i++)
		{
		
			switch (enemystate[i])
		{
		case 0:
			this->enemy[i].setTexture(this->enemytexture[0]);
			break;
		case 1:
			this->enemy[i].setTexture(this->enemytexture[1]);
			this->player.setTexture(this->playertexture[2]);
			if (this->isEndaction(0.5))
			{
				this->enemystate[i] = 0;
				this->playerstate = 0;
			}
			break;
		case 2:
			this->enemy[i].setTexture(this->enemytexture[2]);
			this->player.setTexture(this->playertexture[1]);
			if (this->isEndaction(0.5))
			{
				this->enemystate[i] = 0;
				this->playerstate = 0;
			}
			break;
		}
		}
		
	}
	void UpdateBar(Sprite *target,float currentnum,float maxnum,char type)
	{
		float percent = currentnum / maxnum * 100;
		int ind;
		if (percent <= 0)ind = 10;
		else if (percent <= 10)ind = 9;
		else if (percent <= 20)ind = 8;
		else if (percent <= 30)ind = 7;
		else if (percent <= 40)ind = 6;
		else if (percent <= 50)ind = 5;
		else if (percent <= 60)ind = 4;
		else if (percent <= 70)ind = 3;
		else if (percent <= 80)ind = 2;
		else if (percent < 100)ind = 1;
		else  ind = 0;

		if(type=='H')target->setTexture(this->hpbarTexture[ind]);
		else target->setTexture(this->StaminaTexture[ind]);
	}
	
	void update(const float& dt)
	{
		//update time
		this->Actionclock += dt;
		//enviornment update zone
		this->updateMousePositions();
		this->updateButtons();

		//player update zone
		this->updateInput(dt);

		//this->updateplayer(dt);
		this->UpdatePlayerEnemy(dt);
		

		//update player stat text
		for (int i = 0; i < 5; i++)
		{
		this->playerStatText[i].AlwaysUpdatetext(this->PlayerstatString[i],*this->playerstatusstat[i],dt);
		}
		//update Item remain
		if (isItemwindowActive)
		{
			for (int i = 0; i < 4; i++)
			{
				this->Itemstatus[i].AlwaysUpdatetext("",this->playerstatus.itemnum[i], dt);
			}
		}
		if (isSkillwindowActive)
		{
			for (int i = 0; i < 2; i++)
			{
				this->Skillstatus[i].AlwaysUpdatetext("", this->playerstatus.skillcost[i], dt);
			}
		}

		//update round num text
		this->Roundtext.AlwaysUpdatetext("Round ", (this->round)+1, dt);

		//update bar
		this->UpdateBar(&this->playerhpbar,this->playerstatus.hp,this->playerstatus.max_hp,'H');
		this->UpdateBar(&this->playerstaminabar, this->playerstatus.stamina, this->playerstatus.max_stamina, 'S');
		for(int i=0;i<enemynum;i++)this->UpdateBar(&this->enemyhpbar[i], this->enemystatus[i].hp, this->enemystatus[i].max_hp, 'H');
		// text enemy player stage update 
		this->stagetext.update(dt);
		this->playerdamage.update(dt);
		for (int i = 0; i < enemynum; i++)if(!this->enemystatus[i].isdead or this->enemytext[i].isActive)this->enemytext[i].update(dt);
			
		//is stage end
		if (this->stagetext.isTextshowend and this->isBattleend )
		{
			this->endState();
		}

	}


	void renderButtons(RenderTarget* target = NULL)
	{
		for (auto& it : *currentbutton)
		{
			it.second->render(*target);
		}
	}

	void render(RenderTarget* target = NULL)
	{
		if (!target)
			target = this->window;

		//update environment zone
		target->draw(this->background);

		//render player zone
		if (this->stagetext.isTextshowend)this->player.render(*target);
	
		//render enemy zone
		for (int i = 0; i < enemynum; i++)
		{
			if (!this->enemystatus[i].isdead and this->stagetext.isTextshowend)
			{
				this->enemy[i].render(*target);
				
			}
			if ((!this->enemystatus[i].isdead or this->enemytext[i].isActive)and this->stagetext.isTextshowend)
			{
				target->draw(this->enemytext[i].text);
				target->draw(this->enemyhpbar[i]);
				
			}
			
		}
		//enemy target cursor
		if(this->stagetext.isTextshowend)this->targetCursor.render(*target);
		//render stage text
		target->draw(this->stagetext.text);
		if (this->stagetext.isTextshowend)target->draw(this->Roundtext.text);
		//render other text
		if(playerdamage.isActive)target->draw(this->playerdamage.text);
		
		

		for (int i = 0; i < 5; i++)
		{
			target->draw(this->playerStatText[i].text);
		}
		//render player bar
		target->draw(this->playerhpbar);
		target->draw(this->playerstaminabar);


		//render item window
		if (this->isItemwindowActive)target->draw(this->Itemwindow);
		if (isItemwindowActive)
		{
			for (int i = 0; i < 4;i++)target->draw(Itemstatus[i].text);
		}
		//render skill window
		if (this->isSkillwindowActive)target->draw(this->Skillwindow);
		if (isSkillwindowActive)
		{
			for (int i = 0; i < 2; i++)target->draw(Skillstatus[i].text);
		}
		//render button
		this->renderButtons(target);

		//show mouse position zone
		Text mouseText;
		mouseText.setPosition(this->mousePosView.x, this->mousePosView.y - 20);
		mouseText.setFont(this->font);
		mouseText.setCharacterSize(20);
		std::stringstream ss;
		ss << this->mousePosView.x << " " << this->mousePosView.y;
		mouseText.setString(ss.str());

		target->draw(mouseText);
	}
};

class GameState : public State
{
public:
	//Variables
	Player* player;
	RectangleShape background;
	CollosionBox* box;

	//Initilizer functions
	void initTexture()
	{
		if (!this->textures["PLAYER_SHEET"].loadFromFile("Images/Sprites/Player/test.png"))
			throw "ERROR::GAME_STATE::COULD_NOT_LOAD_PLAYER_TEXTURE";
	}

	void initPlayers()
	{
		this->player = new Player(0, 0, this->textures["PLAYER_SHEET"]);
		this->player->setLight(false);
	}

	void initCollosionBox()
	{
		this->box = new CollosionBox(*this->player, 500.f, 500.f, 0.01f, 0.01f);
	}

	//Constructor / Destructor
	GameState(RenderWindow* window, vector<State*>* states)
		: State(window, states)
	{
		this->background.setFillColor(Color::White);
		this->background.setSize(Vector2f(this->window->getView().getSize()));
		this->initTexture();
		this->initPlayers();
		this->initCollosionBox();
		this->initFonts();
	}

	virtual ~GameState()
	{
		delete this->box;
		delete this->player;
	}

	//Functions
	void updateInput(const float& dt)
	{
		//Update player input
		if (Keyboard::isKeyPressed(Keyboard::W))
			this->player->move(0.f, -1.f, dt);
		else if (Keyboard::isKeyPressed(Keyboard::A))
			this->player->move(-1.f, 0.f, dt);
		else if (Keyboard::isKeyPressed(Keyboard::S))
			this->player->move(0.f, 1.f, dt);
		else if (Keyboard::isKeyPressed(Keyboard::D))
			this->player->move(1.f, 0.f, dt);
		else
			this->player->move(0.f, 0.f, dt);

		if (Keyboard::isKeyPressed(Keyboard::B))
		{
			this->states->push_back(new BattleState(this->window, this->states, 2, 1, false));
		}

		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			this->endState();
		}
	}

	void update(const float& dt)
	{
		this->updateMousePositions();
		this->updateInput(dt);

		if (this->player->getGlobalBounds().left + this->player->getGlobalBounds().width > this->window->getSize().x)
		{
			this->player->setPosition(this->window->getSize().x - this->player->getGlobalBounds().width, this->player->getPosition().y);
		}

		if (this->player->getGlobalBounds().left < 0)
		{
			this->player->setPosition(0, this->player->getPosition().y);
		}

		if (this->player->getGlobalBounds().top + this->player->getGlobalBounds().height > this->window->getSize().y)
		{
			this->player->setPosition(this->player->getPosition().x, this->window->getSize().y - this->player->getGlobalBounds().height);
		}

		if (this->player->getGlobalBounds().top < 0)
		{
			this->player->setPosition(this->player->getPosition().x, 0);
		}

		this->player->update(dt);
		this->box->update();
	}

	void render(RenderTarget* target = NULL)
	{
		if (!target)
			target = this->window;

		target->draw(background);
		this->player->render(*target);
		this->box->render(*target);

		Text mouseText;
		mouseText.setPosition(this->mousePosView.x, this->mousePosView.y - 20);
		mouseText.setFont(this->font);
		mouseText.setCharacterSize(20);
		stringstream ss;
		ss << this->mousePosView.x << " " << this->mousePosView.y;
		mouseText.setString(ss.str());
		target->draw(mouseText);

	}
};

class MainMenuState : public State
{
public:
	//Variables
	Texture backgroundTexture;
	RectangleShape background;

	map<string, Button*> buttons;

	//Initializer functions
	void initVariables()
	{

	}

	void initBackground()
	{

		if (!this->backgroundTexture.loadFromFile("Images/Backgrounds/mainmanu.jpg"))
			throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";

		this->background.setSize(Vector2f(this->window->getView().getSize()));
		this->background.setTexture(&this->backgroundTexture);
	}

	void initButtons()
	{
		this->buttons["GAME_STATE"] = new Button(835, 500, 250, 50, &this->font, "เข้าสู่เกม", 50,
			Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));

		this->buttons["BATTLE_STATE"] = new Button(835, 600, 250, 50, &this->font, "ฝึกต่อสู้", 50,
			Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(70, 70, 70, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));

		this->buttons["EXIT_STATE"] = new Button(835, 700, 250, 50, &this->font, "ออกจากเกม", 50,
			Color(70, 70, 70, 200), Color(250, 250, 250, 250), Color(20, 20, 20, 50),
			Color(100, 100, 100, 0), Color(150, 150, 150, 0), Color(20, 20, 20, 0));
	}

	//Constructor / Destructor
	MainMenuState(RenderWindow* window, vector<State*>* states)
		: State(window, states)
	{
		this->initVariables();
		this->initBackground();
		this->initFonts();
		this->initButtons();
	}

	virtual ~MainMenuState()
	{
		for (auto it = this->buttons.begin(); it != this->buttons.end(); ++it)
		{
			delete it->second;
		}
	}

	//Functions
	void updateButtons()
	{
		/*Updates all the buttons in the state and handles their functionlaity.*/
		for (auto& it : this->buttons)
		{
			it.second->update(this->mousePosView);
		}

		//New game
		if (this->buttons["GAME_STATE"]->isPressed())
		{
			this->states->push_back(new GameState(window, states));
		}

		//Quit the game
		if (this->buttons["EXIT_STATE"]->isPressed())
		{
			this->window->close();
		}
	}

	void update(const float& dt)
	{
		this->updateMousePositions();
		this->updateInput(dt);

		this->updateButtons();
	}

	void renderButtons(RenderTarget& target)
	{
		for (auto& it : this->buttons)
		{
			it.second->render(target);
		}
	}

	void render(RenderTarget* target = NULL)
	{
		if (!target)
			target = this->window;

		target->draw(this->background);

		Text mouseText;
		mouseText.setPosition(this->mousePosView.x, this->mousePosView.y - 20);
		mouseText.setFont(this->font);
		mouseText.setCharacterSize(20);
		stringstream ss;
		ss << this->mousePosView.x << " " << this->mousePosView.y;
		mouseText.setString(ss.str());
		target->draw(mouseText);

		this->renderButtons(*target);
	}
};

Event event;

int main()
{
	bool fullscreen = true;
	unsigned antialiasing_level = 0;
	float speed = 10;
	float dt = 0;

	Clock dtClock;

	ContextSettings windowSettings;
	windowSettings.antialiasingLevel = antialiasing_level;

	RenderWindow* window;
	if (fullscreen)
		window = new RenderWindow(VideoMode(1366, 768), "The Curse", Style::Resize, windowSettings);
	else
		window = new RenderWindow(VideoMode(1920, 1080), "The Curse", Style::Default, windowSettings);
	window->setView(View(FloatRect(0, 0, 1920, 1080)));
	window->setFramerateLimit(60);
	window->setVerticalSyncEnabled(true);

	vector<State*> states;
	states.push_back(new MainMenuState(window, &states));

	while (window->isOpen())
	{
		//Update
		dt = dtClock.restart().asSeconds();

		while (window->pollEvent(event))
		{
			if (event.type == Event::Closed)
				window->close();
		}

		if (!states.empty())
		{
			states.back()->update(dt);

			if (states.back()->getQuit())
			{
				states.back()->endState();
				delete states.back();
				states.pop_back();
			}
		}
		else
		{
			window->close();
		}

		//Render
		window->clear();

		if (!states.empty())
		{
			states.back()->render();
		}

		window->display();
	}

	delete window;

	while (!states.empty())
	{
		delete states.back();
		states.pop_back();
	}

	return 0;
}
