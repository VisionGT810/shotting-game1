# include <Siv3D.hpp> // Siv3D v0.6.16

Vec2 GenerateEnemy()
{
	return RandomVec2({ 50,750 }, -20);
}
void Main()
{
	Scene::SetBackground(ColorF{ 0.1, 0.2, 0.7 });
	const Font font{ FontMethod::MSDF,48 };
	//時機
	const Texture playerTexture{ U"🤖"_emoji };
	//敵
	const Texture enemyTexture{ U"👿"_emoji };
	Vec2 playerPos{ 400,500 };
	Array<Vec2> enemies = { GenerateEnemy() };
	Array<Vec2> playerBullets;
	Array<Vec2> enemyBullets;
	constexpr double PlayerSpeed = 550.0;
	constexpr double PlayerBulletSpeed = 500.0;
	constexpr double EnemySpeed = 100.0;
	constexpr double EnemyBulletSpeed = 300.0;
	constexpr double InitialEnemySpawnInterval = 2.0;
	double enemySpawnTime = InitialEnemySpawnInterval;
	double enemyAccumulatedTime = 0.0;
	constexpr double PlayerShotCoolTime = 0.1;
	double playerShotTimer = 0.0;
	constexpr double EnemyShotCoolTime = 0.9;
	double enemyShotTimer = 0.0;

	Effect effect;

	int32 highScore = 0;
	int32 score = 0;

	while (System::Update())
	{
		bool gameover = false;
		const double deltaTime = Scene::DeltaTime();
		enemyAccumulatedTime += deltaTime;
		playerShotTimer = Min((playerShotTimer + deltaTime), PlayerShotCoolTime);
		enemyShotTimer += deltaTime;
		while (enemySpawnTime <= enemyAccumulatedTime)
		{
			enemyAccumulatedTime -= enemySpawnTime;
			enemySpawnTime = Max(enemySpawnTime * 0.95, 0.3);
			enemies << GenerateEnemy();
		}
		const Vec2 move=Vec2{(KeyRight.pressed()-KeyLeft.pressed()),(KeyDown.pressed()-KeyUp.pressed())}
		.setLength(deltaTime * PlayerSpeed * (KeyShift.pressed() ? 0.5 : 1.0));
		playerPos.moveBy(move).clamp(Scene::Rect());

		if (PlayerShotCoolTime <= playerShotTimer)
		{
			playerShotTimer -= PlayerShotCoolTime;
			playerBullets << playerPos.movedBy(0, -50);
		}
		for (auto& playerBullet : playerBullets)
		{
			playerBullet.y += (deltaTime * -PlayerBulletSpeed);
		}
		playerBullets.remove_if([](const Vec2& b) {return (b.y < -40); });

		for (auto& enemy : enemies)
		{
			enemy.y += (deltaTime * EnemySpeed);
		}
		enemies.remove_if([&](const Vec2& e)
			{
				if (700 < e.y) {
					gameover = true;
					return true;
				}
				else {
					return false;
				}
         });
		if (EnemyShotCoolTime <= enemyShotTimer)
		{
			enemyShotTimer -= EnemyShotCoolTime;
			for (const auto& enemy : enemies)
			{
				enemyBullets << enemy;
			}
		}
		for (auto& enemyBullet : enemyBullets)
		{
			enemyBullet.y += (deltaTime * EnemyBulletSpeed);
		}
		enemyBullets.remove_if([](const Vec2& b) {return(700 < b.y); });

		for (auto itEnemy = enemies.begin(); itEnemy != enemies.end();)
		{
			const Circle enemyCircle{ *itEnemy,40 };
			bool skip = false;
			for (auto itBullet = playerBullets.begin(); itBullet != playerBullets.end();)
			{
				if (enemyCircle.intersects(*itBullet))
				{
					effect.add([pos = *itEnemy](double t)
						{
							const double t2 = ((0.5 - t) * 20);
							Circle{ pos,(10 + t * 280) }.drawFrame((20 * t2), ColorF{ 1.0,(t2 * 0.5) });
							return (t < 0.5);
						});

					itEnemy = enemies.erase(itEnemy);
					playerBullets.erase(itBullet);
					++score;
					skip = true;
					break;
				}
				++itBullet;
			}
			if (skip)
			{
				continue;
			}
			++itEnemy;
		}
		for (const auto& enemyBullet : enemyBullets)
		{
			if (enemyBullet.distanceFrom(playerPos) <= 20)
			{
				gameover = true;
				break;
			}
		}
		if (gameover)
		{
			playerPos = Vec2{ 400,500 };
			enemies.clear();
			playerBullets.clear();
			enemySpawnTime = InitialEnemySpawnInterval;
			highScore = Max(highScore, score);
			score = 0;
		}
		//描画
		for (int32 i = 0; i < 12; ++i)
		{
			const double a = Periodic::Sine0_1(2s, Scene::Time() - (2.0 / 12 * i));
			Rect{ 0,(i * 50),800,50 }.draw(ColorF(1.0, a * 0.2));
		}
		playerTexture.resized(80).flipped().drawAt(playerPos);
		for (const auto& playerBullet : playerBullets)
		{
			Circle{ playerBullet,8 }.draw(Palette::Orange);
		}
		for (const auto& enemy : enemies)
		{
			enemyTexture.resized(60).flipped().drawAt(enemy);
		}
		for (const auto& enemyBullet : enemyBullets)
		{
			Circle{ enemyBullet,4 }.draw(Palette::White);
		}
		effect.update();
		font(U"{} [{}]"_fmt(score, highScore)).draw(30, Arg::bottomRight(780, 580));
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
