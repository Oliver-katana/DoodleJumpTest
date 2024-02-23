//#define _WINDOWS

#include <ctime>
#include <unordered_set>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include "Framework.h"

enum Bonuses
{
	NONE,
	AUTOSHOOT,
	RANDOMSPAWN
};

struct Platforms
{
	Sprite* PlatformSprite;
	float X, Y;
	Sprite* EnemySprite = nullptr;
	Sprite* BonusSprite = nullptr;
	Bonuses BonusType = NONE;
	bool bIsCounted = false;
	bool bIsNested = false;
	bool bIsDestructable = false;
	bool bWithBonus = false;
	float LifeTime = 0;
};

struct Projectile
{
	float DistanceHeroMouse = 0;
	float ProjectileX = 0, ProjectileY = 0;
	float DeltaProjectileX = 0, DeltaProjectileY = 0;
	float LifeTime = 0;
	short PrjctlAssignedNumber = -1;
	Sprite* ProjectileSprite = nullptr;
};

class MyFramework : public Framework
{
	std::vector<Platforms> PlatformArray;

	std::list<Projectile> ProjectileArray;

	std::unordered_set<short> EnemyArray;

	std::unordered_set<short> BonusArray;

	std::unordered_set<short> ProjectileAssignedArray;

	int WindowWidth = 0, WindowHeight = 0;
	int ScreenSizeX = 0, ScreenSizeY = 0;
	int HeroSpriteSizeX = 0, HeroSpriteSizeY = 0;

	//Sprites variables
	//--------------------------------------------------------------------------
	Sprite* Digits[10]{};
	Sprite* HeroSprite = nullptr, * HeroSpriteLeft = nullptr, * HeroSpriteRight = nullptr, * EnemySprite = nullptr,
		* PlatformSprite = nullptr, * PlatformSpriteStep1 = nullptr, * PlatformSpriteStep2 = nullptr, * PlatformSpriteStep3 = nullptr,
		* ProjectileSprite = nullptr, * PlatformsScoreSprite = nullptr, * DistanceScoreSprite = nullptr, * AutoShootSprite = nullptr,
		* BonusAutoShootSprite = nullptr, * RandomSpawnSprite = nullptr, * BonusRandSpawnSprite = nullptr, * ReadySprite = nullptr;
	//--------------------------------------------------------------------------

	//Hero variables
	//--------------------------------------------------------------------------
	const float DELTA = -2.5f;
	const float PROJECTILE_SPEED = 2.0f;
	const short JUMPS = 50;

	Bonuses HeroBonusType = NONE;
	float DeltaMoveX = 0.0f, DeltaMoveY = 0.0f;
	float HeroSpeed = 1.0f;
	float HeroX = 0.0f, HeroY = 0.0f;
	float HeroBonusTime = 0.0f;
	float PlatformDeltaY = 0.0f;
	bool bMouseRightPressed = false;
	bool bIsShooting = false;
	short HeroJumpsValue = 0;
	//--------------------------------------------------------------------------

	// Mouse Coordinates
	//--------------------------------------------------------------------------
	float MouseX = 0, MouseY = 0;
	//--------------------------------------------------------------------------

	//Score variables
	//--------------------------------------------------------------------------
	int PlatformScore = 0, DistanceScore = 0;
	//--------------------------------------------------------------------------

	//Time variable
	//--------------------------------------------------------------------------
	time_t  ShootSeconds = 0, CurrentTimeSeconds = 0;
	//--------------------------------------------------------------------------

public:
	MyFramework(int width, int height)
	{
		WindowWidth = width; WindowHeight = height;
	}

	virtual void PreInit(int& width, int& height, bool& fullscreen)
	{
		width = WindowWidth;
		height = WindowHeight;
		fullscreen = false;
	}

	virtual bool Init()
	{
		// Assign sprites
		//--------------------------------------------------------------------------
		HeroSprite = createSprite("Sprites/HeroDown.png");
		HeroSpriteLeft = createSprite("Sprites/HeroLeft.png");
		HeroSpriteRight = createSprite("Sprites/HeroRight.png");
		PlatformsScoreSprite = createSprite("Sprites/Platforms.png");
		DistanceScoreSprite = createSprite("Sprites/Distance.png");
		PlatformSprite = createSprite("Sprites/Platform.png");
		PlatformSpriteStep1 = createSprite("Sprites/PlatformStep1.png");
		PlatformSpriteStep2 = createSprite("Sprites/PlatformStep2.png");
		PlatformSpriteStep3 = createSprite("Sprites/PlatformStep3.png");
		EnemySprite = createSprite("Sprites/Enemy.png");
		ProjectileSprite = createSprite("Sprites/Projectile.png");
		AutoShootSprite = createSprite("Sprites/AutoShoot.png");
		BonusAutoShootSprite = createSprite("Sprites/BonusAutoShoot.png");
		RandomSpawnSprite = createSprite("Sprites/RandomSpawn.png");
		BonusRandSpawnSprite = createSprite("Sprites/RandJump.png");
		ReadySprite = createSprite("Sprites/Ready.png");

		for (size_t i = 0; i < 10; i++)
		{
			char DigitPath[20] = "Sprites/Digit";
			strncat_s(DigitPath, std::to_string(i).c_str(), 1);
			strncat_s(DigitPath, ".png", 4);
			Digits[i] = createSprite(DigitPath);
		}
		//--------------------------------------------------------------------------

		getScreenSize(ScreenSizeX, ScreenSizeY);
		getSpriteSize(HeroSprite, HeroSpriteSizeX, HeroSpriteSizeY);

		StartRestartGame();

		return true;
	}

	virtual void Close()
	{
	}

	virtual bool Tick()
	{
		CurrentTimeSeconds = time(NULL) - 1700000000;
		MyFramework::GetTitle();
		drawTestBackground();

		//Moving hero - axis X
		//--------------------------------------------------------------------------
		HeroX += DeltaMoveX * HeroSpeed;

		if (HeroX + HeroSpriteSizeX / 2 > ScreenSizeX)
		{
			HeroX = 0.0f - HeroSpriteSizeX / 2;
		}
		if (HeroX < (0.0f - HeroSpriteSizeX / 2))
		{
			HeroX = ScreenSizeX - HeroSpriteSizeX / 2;
		}
		//--------------------------------------------------------------------------

		//Increase - decrease hero speed in depends of window side - at right side moving on 50% faster
		//--------------------------------------------------------------------------
		if (HeroX + 31 > ScreenSizeX / 2)
		{
			HeroSpeed = 1.5f;
		}
		else
		{
			HeroSpeed = 1.0f;
		}
		//--------------------------------------------------------------------------

		//Jumping hero - axis Y
		//--------------------------------------------------------------------------
		if (PlatformDeltaY <= 0)
		{
			DeltaMoveY += 0.02f;
			HeroY += DeltaMoveY;
		}
		//--------------------------------------------------------------------------

		//Check hero collision with bonus
		//--------------------------------------------------------------------------
		for (auto itBonus = BonusArray.begin(); itBonus != BonusArray.end(); ++itBonus)
		{
			if ((HeroX + 52 > PlatformArray[*itBonus].X + 9) && (HeroX < PlatformArray[*itBonus].X + 39) &&
				(HeroY + 50 > PlatformArray[*itBonus].Y - 40) && (HeroY < PlatformArray[*itBonus].Y - 10))
			{
				HeroBonusType = PlatformArray[*itBonus].BonusType;
				HeroBonusTime = CurrentTimeSeconds;
				HeroJumpsValue = 0;
				PlatformArray[*itBonus].bWithBonus = false;
				itBonus = BonusArray.erase(itBonus);
				break;
			}
		}
		//--------------------------------------------------------------------------

		//Check hero collision with enemy
		//--------------------------------------------------------------------------
		for (auto itEnemy = EnemyArray.begin(); itEnemy != EnemyArray.end(); ++itEnemy)
		{
			if ((HeroX + 52 > PlatformArray[*itEnemy].X + 9) && (HeroX < PlatformArray[*itEnemy].X + 39) &&
				(HeroY + 60 > PlatformArray[*itEnemy].Y - 40) && (HeroY + 60 < PlatformArray[*itEnemy].Y - 30) && DeltaMoveY > 0)
			{
				PlatformArray[*itEnemy].bIsNested = false;
				itEnemy = EnemyArray.erase(itEnemy);
				DeltaMoveY = DELTA;
				if (HeroBonusType == RANDOMSPAWN) { ++HeroJumpsValue; }
				break;
			}
			else if ((HeroX + 52 > PlatformArray[*itEnemy].X + 9) && (HeroX < PlatformArray[*itEnemy].X + 39) &&
				(HeroY + 50 > PlatformArray[*itEnemy].Y - 40) && (HeroY < PlatformArray[*itEnemy].Y - 10))
			{
				StartRestartGame();
				break;
			}
		}
		//--------------------------------------------------------------------------

		//Check if fall down
		//--------------------------------------------------------------------------
		if (HeroY > ScreenSizeY)
		{
			StartRestartGame();
		}
		//--------------------------------------------------------------------------

		//Moving up by platforms and drawing new platforms
		//--------------------------------------------------------------------------
		if (HeroY < (ScreenSizeY / 3 * 2 - 20))
		{
			if (PlatformDeltaY > 0)
			{
				HeroY += 0.3;
				PlatformDeltaY -= 0.3;
			}
			else
			{
				HeroY = ScreenSizeY / 3 * 2 - 20;
			}

			DistanceScore -= DeltaMoveY;

			for (size_t i = 0; i < PlatformArray.size(); ++i)
			{
				if (PlatformDeltaY > 0)
				{
					PlatformArray[i].Y += 0.3;
				}
				else
				{
					PlatformArray[i].Y -= DeltaMoveY;
				}

				//Renew platforms
				//--------------------------------------------------------------------------
				if (PlatformArray[i].Y > ScreenSizeY)
				{
					PlatformArray[i].X = rand() % (ScreenSizeX - 58); PlatformArray[i].Y = 0;
					PlatformArray[i].PlatformSprite = PlatformSprite;
					PlatformArray[i].bIsCounted = false;
					PlatformArray[i].bIsDestructable = false;
					PlatformArray[i].bIsNested = false;
					PlatformArray[i].bWithBonus = false;
					EnemyArray.erase(short(i));

					//Create destrucrable platforms
					//--------------------------------------------------------------------------
					if (rand() % 5 == 0)
					{
						PlatformArray[i].bIsDestructable = true;
						PlatformArray[i].LifeTime = time(NULL) - 1700000000;
						PlatformArray[i].PlatformSprite = PlatformSpriteStep1;
					}
					//--------------------------------------------------------------------------

					//Create platforms with enemy
					//--------------------------------------------------------------------------
					if (!PlatformArray[i].bIsDestructable && rand() % 8 == 0)
					{
						PlatformArray[i].bIsNested = true;
						PlatformArray[i].EnemySprite = EnemySprite;
						EnemyArray.insert(short(i));
					}
					//--------------------------------------------------------------------------

					//Create platforms with bonus
					//--------------------------------------------------------------------------
					if (!PlatformArray[i].bIsDestructable && !PlatformArray[i].bIsNested && rand() % 20 == 0)
					{
						PlatformArray[i].bWithBonus = true;
						BonusArray.insert(short(i));
						switch (rand() % 2 + 1)
						{
						case AUTOSHOOT:
						{
							PlatformArray[i].BonusType = AUTOSHOOT;
							PlatformArray[i].BonusSprite = AutoShootSprite;
							break;
						}
						case RANDOMSPAWN:
						{
							PlatformArray[i].BonusType = RANDOMSPAWN;
							PlatformArray[i].BonusSprite = RandomSpawnSprite;
							break;
						}
						default:
							break;
						}
					}
					//--------------------------------------------------------------------------
				}
				//--------------------------------------------------------------------------
			}
		}
		//--------------------------------------------------------------------------
		//Platforms
		//--------------------------------------------------------------------------
		for (size_t i = 0; i < PlatformArray.size(); ++i)
		{
			//Check platform collision with hero
			//--------------------------------------------------------------------------
			if ((HeroX + 40 > PlatformArray[i].X) && (HeroX < PlatformArray[i].X + 40) &&
				(HeroY + 60 > PlatformArray[i].Y - 1) && (HeroY + 60 < PlatformArray[i].Y + 5) && (DeltaMoveY > 0))
			{
				DeltaMoveY = DELTA;
				if (HeroBonusType == RANDOMSPAWN) { ++HeroJumpsValue; }
			}
			//--------------------------------------------------------------------------

			//Count platforms passed
			//--------------------------------------------------------------------------
			if (!PlatformArray[i].bIsCounted && PlatformArray[i].Y > ScreenSizeY - 30)
			{
				++PlatformScore;
				PlatformArray[i].bIsCounted = true;
			}
			//--------------------------------------------------------------------------

			//Handle destructable platforms
			//--------------------------------------------------------------------------
			if (PlatformArray[i].bIsDestructable)
			{
				if (CurrentTimeSeconds - PlatformArray[i].LifeTime == 2)
				{
					PlatformArray[i].PlatformSprite = PlatformSpriteStep2;
				}
				else if (CurrentTimeSeconds - PlatformArray[i].LifeTime == 4)
				{
					PlatformArray[i].PlatformSprite = PlatformSpriteStep3;
				}
				else if (CurrentTimeSeconds - PlatformArray[i].LifeTime == 6)
				{
					PlatformArray[i].bIsDestructable = false;
					PlatformArray[i].X = rand() % (ScreenSizeX - 58); PlatformArray[i].Y = -rand() % ScreenSizeY;
					PlatformArray[i].PlatformSprite = PlatformSprite;
				}
			}
			//--------------------------------------------------------------------------

			//Draw platforms and enemy if nested and bonus if has bonus
			//--------------------------------------------------------------------------
			if (PlatformArray[i].Y > 0)
			{
				drawSprite(PlatformArray[i].PlatformSprite, PlatformArray[i].X, PlatformArray[i].Y);
			}
			if (PlatformArray[i].bIsNested)
			{
				drawSprite(PlatformArray[i].EnemySprite, PlatformArray[i].X + 9, PlatformArray[i].Y - 40);
			}
			if (PlatformArray[i].bWithBonus)
			{
				drawSprite(PlatformArray[i].BonusSprite, PlatformArray[i].X + 9, PlatformArray[i].Y - 40);
			}
			//--------------------------------------------------------------------------
		}
		//--------------------------------------------------------------------------

		//Draw score - Distance passed, Platforms passed
		//--------------------------------------------------------------------------
		drawSprite(DistanceScoreSprite, ScreenSizeX - 110, 20);
		drawSprite(PlatformsScoreSprite, ScreenSizeX - 110, 40);
		int TempDistanceScore = DistanceScore, TempPlaformScore = PlatformScore;
		int spacer = 1;
		while (TempDistanceScore)
		{
			drawSprite(Digits[TempDistanceScore % 10], ScreenSizeX - 110 - (13 * spacer++), 20);
			TempDistanceScore /= 10;
		}
		spacer = 1;
		while (TempPlaformScore)
		{
			drawSprite(Digits[TempPlaformScore % 10], ScreenSizeX - 110 - (13 * spacer++), 40);
			TempPlaformScore /= 10;
		}
		//--------------------------------------------------------------------------

		//Shoot
		//--------------------------------------------------------------------------
		if (CurrentTimeSeconds - ShootSeconds > 1)
		{
			bIsShooting = false;
		}
		for (auto itPrjctl = ProjectileArray.begin(); itPrjctl != ProjectileArray.end(); ++itPrjctl)
		{
			//Check if enemy is shut
			//--------------------------------------------------------------------------
			for (auto itEnemy = EnemyArray.begin(); itEnemy != EnemyArray.end(); ++itEnemy)
			{
				if (itPrjctl->ProjectileX + 7 > PlatformArray[*itEnemy].X + 9 && itPrjctl->ProjectileX + 7 < PlatformArray[*itEnemy].X + 49 &&
					itPrjctl->ProjectileY + 7 > PlatformArray[*itEnemy].Y - 40 && itPrjctl->ProjectileY + 7 < PlatformArray[*itEnemy].Y)
				{
					PlatformArray[*itEnemy].bIsNested = false;
					itEnemy = EnemyArray.erase(itEnemy);
					if (itPrjctl->PrjctlAssignedNumber >= 0)
					{
						ProjectileAssignedArray.erase(itPrjctl->PrjctlAssignedNumber);
					}
					itPrjctl = ProjectileArray.erase(itPrjctl);
					bIsShooting = false;
					break;
				}
			}
			if (itPrjctl == ProjectileArray.end()) { break; }
			//--------------------------------------------------------------------------

			//Check if projectile out of borders or lifetime is up
			//--------------------------------------------------------------------------
			if (itPrjctl->ProjectileX + 7 > ScreenSizeX) { itPrjctl->ProjectileX = 0 - 7; }
			if (itPrjctl->ProjectileX < 0 - 7) { itPrjctl->ProjectileX = ScreenSizeX - 7; }
			if (itPrjctl->ProjectileY < 0 || itPrjctl->ProjectileY > ScreenSizeY || CurrentTimeSeconds - itPrjctl->LifeTime > 3)
			{
				if (itPrjctl->PrjctlAssignedNumber >= 0)
				{
					ProjectileAssignedArray.erase(itPrjctl->PrjctlAssignedNumber);
				}
				itPrjctl = ProjectileArray.erase(itPrjctl);
				bIsShooting = false;
				if (itPrjctl == ProjectileArray.end()) { break; }
			}
			else
			{
				drawSprite(itPrjctl->ProjectileSprite, itPrjctl->ProjectileX, itPrjctl->ProjectileY);
			}
			//--------------------------------------------------------------------------

			itPrjctl->ProjectileY += itPrjctl->DeltaProjectileY;
			itPrjctl->ProjectileX += itPrjctl->DeltaProjectileX;
		}
		//--------------------------------------------------------------------------

		//Handle bonus
		//--------------------------------------------------------------------------
		switch (HeroBonusType)
		{
		case AUTOSHOOT:
		{
			drawSprite(BonusAutoShootSprite, 10, 20);
			spacer = 1;
			int TempTimeBonus = 20 - (CurrentTimeSeconds - HeroBonusTime);
			while (TempTimeBonus > 0)
			{
				drawSprite(Digits[TempTimeBonus % 10], 150 - (13 * spacer++), 20);
				TempTimeBonus /= 10;
			}
			if (CurrentTimeSeconds - HeroBonusTime < 20)
			{
				for (auto itEnemy = EnemyArray.begin(); itEnemy != EnemyArray.end(); ++itEnemy)
				{
					if (ProjectileAssignedArray.find(*itEnemy) == ProjectileAssignedArray.end())
					{
						ProjectileAssignedArray.insert(*itEnemy);
						Projectile projectile;
						projectile.DistanceHeroMouse = hypot(HeroX - (PlatformArray[*itEnemy].X + 29), HeroY - (PlatformArray[*itEnemy].Y - 20));
						projectile.ProjectileX = HeroX + HeroSpriteSizeX / 2;
						projectile.ProjectileY = HeroY;
						projectile.DeltaProjectileX = ((PlatformArray[*itEnemy].X + 29) - projectile.ProjectileX) / projectile.DistanceHeroMouse * PROJECTILE_SPEED;
						projectile.DeltaProjectileY = ((PlatformArray[*itEnemy].Y - 20) - projectile.ProjectileY) / projectile.DistanceHeroMouse * PROJECTILE_SPEED;
						projectile.LifeTime = time(NULL) - 1700000000;
						projectile.ProjectileSprite = ProjectileSprite;
						projectile.PrjctlAssignedNumber = *itEnemy;
						ProjectileArray.push_back(projectile);
					}
				}
			}
			else
			{
				HeroBonusType = NONE;
			}
			break;
		}
		case RANDOMSPAWN:
		{
			drawSprite(BonusRandSpawnSprite, 10, 20);
			spacer = 1;
			int TempJumps = JUMPS - HeroJumpsValue;
			while (TempJumps > 0)
			{
				drawSprite(Digits[TempJumps % 10], 140 - (13 * spacer++), 20);
				TempJumps /= 10;
			}
			if (HeroJumpsValue >= JUMPS)
			{
				drawSprite(ReadySprite, 120, 20);
				if (bMouseRightPressed)
				{
					short RandomPosition = rand() % PlatformArray.size();
					PlatformDeltaY = (ScreenSizeY / 3 * 2 - 20 - PlatformArray[RandomPosition].Y) *
						((ScreenSizeY / 3 * 2 - 20 - PlatformArray[RandomPosition].Y) > 0);
					HeroX = PlatformArray[RandomPosition].X - 2;
					HeroY = PlatformArray[RandomPosition].Y - 60;

					HeroBonusType = NONE;
					HeroJumpsValue = 0;
					bMouseRightPressed = false;
				}
			}
			break;
		}
		default:
		{
			break;
		}
		}
		//--------------------------------------------------------------------------

		//Draw hero
		//--------------------------------------------------------------------------
		drawSprite(HeroSprite, HeroX, HeroY);
		//--------------------------------------------------------------------------

		return false;
	}

	virtual void onMouseMove(int x, int y, int xrelative, int yrelative)
	{
		MouseX = x; MouseY = y;
	}

	virtual void onMouseButtonClick(FRMouseButton button, bool isReleased)
	{
		switch (button)
		{
		case FRMouseButton::LEFT:
		{
			if (!isReleased && !bIsShooting)
			{
				bIsShooting = true;
				ShootSeconds = time(NULL) - 1700000000;

				Projectile projectile;
				projectile.DistanceHeroMouse = hypot(HeroX - MouseX, HeroY - MouseY);
				projectile.ProjectileX = HeroX + HeroSpriteSizeX / 2;
				projectile.ProjectileY = HeroY;
				projectile.DeltaProjectileX = (MouseX - projectile.ProjectileX) / projectile.DistanceHeroMouse * PROJECTILE_SPEED;
				projectile.DeltaProjectileY = (MouseY - projectile.ProjectileY) / projectile.DistanceHeroMouse * PROJECTILE_SPEED;
				projectile.LifeTime = ShootSeconds;
				projectile.ProjectileSprite = ProjectileSprite;
				ProjectileArray.push_back(projectile);
			}
			break;
		}
		case FRMouseButton::RIGHT:
		{
			if (!isReleased && HeroJumpsValue >= JUMPS)
			{
				bMouseRightPressed = true;
			}
			break;
		}
		default:
			break;
		}
	}

	virtual void onKeyPressed(FRKey k)
	{
		switch (k)
		{
		case FRKey::RIGHT:
		{
			HeroSprite = HeroSpriteRight;
			DeltaMoveX = 0.3;
			break;
		}
		case FRKey::LEFT:
		{
			HeroSprite = HeroSpriteLeft;
			DeltaMoveX = -0.3;
			break;
		}
		case FRKey::DOWN:
			StartRestartGame();
			break;
		default:
			break;
		}
	}

	virtual void onKeyReleased(FRKey k)
	{
		DeltaMoveX = 0;
	}

	virtual const char* GetTitle() override
	{
		return "Doodle Jump by Oliver";
	}

	void StartRestartGame()
	{
		//Clear data for restart
		//--------------------------------------------------------------------------
		PlatformArray.clear();
		ProjectileArray.clear();
		EnemyArray.clear();
		BonusArray.clear();
		ProjectileAssignedArray.clear();
		PlatformDeltaY = 0;
		HeroJumpsValue = 0;
		HeroBonusType = NONE;
		bMouseRightPressed = false;
		//--------------------------------------------------------------------------

		//Init hero start position
		//--------------------------------------------------------------------------
		HeroX = ScreenSizeX / 2 - HeroSpriteSizeX / 2;
		HeroY = ScreenSizeY - 80;
		//--------------------------------------------------------------------------

		//Create platforms
		//--------------------------------------------------------------------------
		for (size_t i = 0; i < ScreenSizeX / 58; ++i)
		{
			Platforms Platform{ createSprite("Sprites/Platform.png"), int(i) * 58, ScreenSizeY - 20 };
			PlatformArray.push_back(Platform);
		}
		for (size_t i = 0; i < ScreenSizeX * ScreenSizeY / 10000; ++i)
		{
			Platforms Platform{ createSprite("Sprites/Platform.png"), rand() % (ScreenSizeX - 58),rand() % (ScreenSizeY - 40) };
			PlatformArray.push_back(Platform);
		}
		//--------------------------------------------------------------------------

		//Restart score
		//--------------------------------------------------------------------------
		PlatformScore = 0, DistanceScore = 0;
		//--------------------------------------------------------------------------
	}
};

int main(int argc, char* argv[])
{

	srand(time(0));

	int ScreenWidth = 480, ScreenHeight = 640;
	if (argc == 3)
	{
		std::string ResArg(argv[1]);
		if (ResArg == "-window")
		{
			std::stringstream Resolution(argv[2]);
			Resolution >> ScreenWidth;
			Resolution.get();
			Resolution >> ScreenHeight;
		}
	}

	return run(new MyFramework(ScreenWidth, ScreenHeight));
}
