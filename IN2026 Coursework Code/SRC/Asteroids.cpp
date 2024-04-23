#include <fstream>
#include <iostream>
#include <iomanip>
#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "DemoSpaceship.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mGameStarted = false;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	// Reads the high scores from the text file
	ReadHighScoresFromFile();

	// Create a spaceship and add it to the world
	mGameWorld->AddObject(CreateSpaceship()); // Comment out when changing to demo spaceship

	// Create a demo spaceship and add it to the world
	mGameWorld->AddObject(CreateDemoSpaceship());
	SetTimer(500, DEMOSPACESHIP_SHOOT);

	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// Reads from the HighScores.txt file
void Asteroids::ReadHighScoresFromFile()
{
	int HighScoresFromFile;
	ifstream ifs;
	ifs.open("HighScores.txt");
	if (ifs.fail()) {
		// Print in the terminal if the file fails to open
		cout << "File failed to open" << endl;
	}
	else {
		// Print in the terminal if the file successfully opens
		cout << "File opened successfully" << endl;

		// Reads the first three high scores in order of highest to lowest
		ifs >> HighScoresFromFile;
		mHighScoreTopFromFile = HighScoresFromFile;
		ifs >> HighScoresFromFile;
		mHighScoreMidFromFile = HighScoresFromFile;
		ifs >> HighScoresFromFile;
		mHighScoreBotFromFile = HighScoresFromFile;
	}
	ifs.close();
}

//Saves scores to the HighScores.txt file
void Asteroids::SaveHighScoresToFile()
{
	ofstream fout;
	fout.open("HighScores.txt");
	fout << mHighScoreTopFromFile << endl;
	fout << mHighScoreMidFromFile << endl;
	fout << mHighScoreBotFromFile << endl;
	fout.close();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (!mGameStarted)
	{
		switch (key)
		{
		case ' ':
			// Create a spaceship and add it to the world
			mGameWorld->AddObject(CreateSpaceship());

			mGameStarted = true;
			mStartGameLabel->SetVisible(false);
			mLivesLabel->SetVisible(true);
			mScoreLabel->SetVisible(true);

			// Remove demo spaceship from the world
			mGameWorld->FlagForRemoval(mDemoSpaceship);

			// Reset the lives and score from the demo
			mScoreKeeper.mScore = 0;
			mPlayer.mLives = 3;

			break;
		default:
			break;
		}
	}
	switch (key)
	{
	case ' ':
		mSpaceship->Shoot();
		break;
	default:
		break;
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
	// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
	// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		// Creates the two smaller asteroids when the original asteroid is hit by a bullet
		if (object->GetScale() > 0.1f)
		{
			CreateSmallerAsteroids(2, object->GetPosition());
		}
		if (mAsteroidCount <= 0) 
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
	}
	if (object->GetType() == GameObjectType("DemoSpaceship"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(mDemoSpaceship->GetPosition());
		explosion->SetRotation(mDemoSpaceship->GetRotation());
		mGameWorld->AddObject(explosion);
		SetTimer(500, DEMOSPACESHIP_RESPAWN);
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
		mHighScoreLabel->SetVisible(true);
		mHighScoreTopLabel->SetVisible(true);
		mHighScoreMidLabel->SetVisible(true);
		mHighScoreBotLabel->SetVisible(true);

		if (mHighScoreTopFromFile < mCurrentScore) {
			std::ostringstream h_msg_stream;
			h_msg_stream << "1st: New High Score: " << mCurrentScore;
			mHighScoreBotFromFile = mHighScoreMidFromFile;
			mHighScoreMidFromFile = mHighScoreTopFromFile;
			mHighScoreTopFromFile = mCurrentScore;
			std::string h_score_msg = h_msg_stream.str();
			mHighScoreTopLabel->SetText(h_score_msg);
			RefreshHighScores(mHighScoreMidLabel, "2nd: Score: " + std::to_string(mHighScoreMidFromFile));
			RefreshHighScores(mHighScoreBotLabel, "3rd: Score: " + std::to_string(mHighScoreBotFromFile));
		}
		else if (mHighScoreMidFromFile < mCurrentScore) {
			std::ostringstream h_msg_stream;
			h_msg_stream << "2nd: Score: " << mCurrentScore;
			mHighScoreBotFromFile = mHighScoreMidFromFile;
			mHighScoreMidFromFile = mCurrentScore;
			std::string h_score_msg = h_msg_stream.str();
			mHighScoreMidLabel->SetText(h_score_msg);
			RefreshHighScores(mHighScoreBotLabel, "3rd: Score: " + std::to_string(mHighScoreBotFromFile));
		}
		else if (mHighScoreBotFromFile < mCurrentScore) {
			std::ostringstream h_msg_stream;
			h_msg_stream << "3rd: Your Score: " << mCurrentScore;
			mHighScoreBotFromFile = mCurrentScore;
			std::string h_score_msg = h_msg_stream.str();
			mHighScoreBotLabel->SetText(h_score_msg);
		}
		//else if (mHighScoreTopFromFile == mCurrentScore || mHighScoreMidFromFile == mCurrentScore || mHighScoreBotFromFile == mCurrentScore) {
		//	cout << "Did not beat an existing score" << endl;
		//}
		SaveHighScoresToFile();
	}
	if (value == DEMOSPACESHIP_SHOOT)
	{
		if (!mGameStarted)
		{
			mDemoSpaceship->Thrust(rand() % 10 + (2));
			mDemoSpaceship->Rotate(rand() % 120 + (-100));
			mDemoSpaceship->Shoot();
			SetTimer(600, DEMOSPACESHIP_SHOOT);
		}
	}

	if (value == DEMOSPACESHIP_RESPAWN)
	{
		if (!mGameStarted) {
			mGameWorld->AddObject(CreateDemoSpaceship());
		}
	}
}

void Asteroids::RefreshHighScores(shared_ptr<GUILabel> RefreshGUILabel, string value) {
	std::ostringstream h_msg_stream;
	h_msg_stream << value;
	std::string h_score_msg = h_msg_stream.str();
	RefreshGUILabel->SetText(h_score_msg);
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

shared_ptr<GameObject> Asteroids::CreateDemoSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mDemoSpaceship = make_shared<DemoSpaceship>();
	mDemoSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mDemoSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mDemoSpaceship->SetDemoBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mDemoSpaceship->SetSprite(spaceship_sprite);
	mDemoSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mDemoSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mDemoSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

// Creates smaller asteroids that are half the size of the original asteroid
void Asteroids::CreateSmallerAsteroids(const uint num_asteroids, GLVector3f p)
{
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid_small = make_shared<Asteroid>();
		asteroid_small->SetBoundingShape(make_shared<BoundingSphere>(asteroid_small->GetThisPtr(), 5.0f));
		asteroid_small->SetSprite(asteroid_sprite);
		asteroid_small->SetScale(0.1f);
		asteroid_small->SetPosition(p);
		mGameWorld->AddObject(asteroid_small);
	}
}



void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel for ending the game and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);

	// Create a new GUILabel for starting the game and wrap it up in a shared_ptr
	mStartGameLabel = make_shared<GUILabel>("PRESS SPACE TO START");
	// Set the horizontal alignment of the label to GUI_VALIGN_CENTER
	mStartGameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mStartGameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (true)
	mStartGameLabel->SetVisible(true);

	mHighScoreLabel = shared_ptr<GUILabel>(new GUILabel("High Scores"));
	mHighScoreLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	mHighScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreLabel->SetVisible(false);

	mHighScoreTopLabel = shared_ptr<GUILabel>(new GUILabel("1st: Score: " + std::to_string(mHighScoreTopFromFile)));
	mHighScoreTopLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	mHighScoreTopLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreTopLabel->SetVisible(false);

	mHighScoreMidLabel = shared_ptr<GUILabel>(new GUILabel("2nd: Score: " + std::to_string(mHighScoreMidFromFile)));
	mHighScoreMidLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	mHighScoreMidLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreMidLabel->SetVisible(false);

	mHighScoreBotLabel = shared_ptr<GUILabel>(new GUILabel("3rd: Score: " + std::to_string(mHighScoreBotFromFile)));
	mHighScoreBotLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
	mHighScoreBotLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mHighScoreBotLabel->SetVisible(false);

	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

	shared_ptr<GUIComponent> start_demo_component 
		= static_pointer_cast<GUIComponent>(mStartGameLabel);
	mGameDisplay->GetContainer()->AddComponent(start_demo_component, GLVector2f(0.5f, 0.7f));

	shared_ptr<GUIComponent> high_score_component
		= static_pointer_cast<GUIComponent>(mHighScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(high_score_component, GLVector2f(0.35f, 0.7f));
	shared_ptr<GUIComponent> high_score_component_top
		= static_pointer_cast<GUIComponent>(mHighScoreTopLabel);
	mGameDisplay->GetContainer()->AddComponent(high_score_component_top, GLVector2f(0.35f, 0.65f));
	shared_ptr<GUIComponent> high_score_component_mid
		= static_pointer_cast<GUIComponent>(mHighScoreMidLabel);
	mGameDisplay->GetContainer()->AddComponent(high_score_component_mid, GLVector2f(0.35f, 0.6f));
	shared_ptr<GUIComponent> high_score_component_bot
		= static_pointer_cast<GUIComponent>(mHighScoreBotLabel);
	mGameDisplay->GetContainer()->AddComponent(high_score_component_bot, GLVector2f(0.35f, 0.55f));
}

void Asteroids::OnScoreChanged(int score)
{
	mCurrentScore = score;
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		SetTimer(1000, CREATE_NEW_PLAYER); 
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}