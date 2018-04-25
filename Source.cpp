#include <fstream>
#include <string>
#include "XLibrary11.hpp"
using namespace std;
using namespace XLibrary11;

enum Mode
{
	Title,
	Game,
	Clear,
	Over,
};

struct Character
{
	Float2 position = 0.0f;
	Float2 velocity = 0.0f;
	Float2 scale = 2.0f;
	bool isGround = false;
	int side = 1;
};

const char playerID = 'P';
const char enemyID = 'E';
const char airID = '.';
const char blockID = '#';
const char coinID = '0';
const char luckyID = '?';
const char goalID = '@';

Mode mode = Title;

void UpdatePlayer(Character& player, vector<Character>& enemys, vector<char>& map, int width, int height)
{
	static Sound jumpSound(L"jump.wav");
	static Sound hitSound(L"hit.wav");
	static Sound coinSound(L"coin.wav");
	static Sound enemySound(L"enemy.wav");
	static Sound clearSound(L"clear.mp3");
	static Sound overSound(L"over.mp3");

	float speed = 1.0f;
	if (App::GetKey('X'))
	{
		speed = 2.0f;
	}

	if (!player.isGround)
	{
		speed *= 0.3f;
	}

	if (App::GetKey(VK_RIGHT))
	{
		player.velocity.x += speed;
		player.scale.x = 2.0f;
	}
	if (App::GetKey(VK_LEFT))
	{
		player.velocity.x -= speed;
		player.scale.x = -2.0f;
	}
	if (App::GetKeyDown('Z'))
	{
		if (player.isGround)
		{
			player.velocity.y = 12.0f;
			jumpSound.Play();
		}
	}
	if (App::GetKeyUp('Z'))
	{
		if (player.velocity.y > 0.0f)
			player.velocity.y *= 0.5f;
	}

	player.velocity.x *= 0.9f;
	player.velocity.y -= 0.5f;

	if (player.velocity.y < -15.0f)
		player.velocity.y = -15.0f;

	player.isGround = false;

	int centerX = (int)roundf(player.position.x / 32.0f);
	int centerY = height - (int)roundf(player.position.y / 32.0f);

	for (int x = centerX - 1; x <= centerX + 1; x++)
	{
		for (int y = centerY - 1; y <= centerY + 1; y++)
		{
			if (x < 0 || y < 0 || x >= width || y >= height)
				continue;

			Float2 now = player.position;
			Float2 next = player.position + player.velocity;
			Float2 hit = Float2(x * 32.0f, (height - y) * 32.0f);

			if (map[x + y * width] == blockID)
			{
				if (next.x > hit.x - 32 &&
					next.x < hit.x + 32 &&
					now.y > hit.y - 32 &&
					now.y < hit.y + 32)
				{
					player.velocity.x = roundf(next.x / 32.0f) * 32.0f - now.x;
				}

				if (now.x > hit.x - 32 &&
					now.x < hit.x + 32 &&
					next.y > hit.y - 32 &&
					next.y < hit.y + 32)
				{
					if (player.velocity.y > 0.0f)
					{
						hitSound.Play();
					}

					player.isGround = true;
					player.velocity.y = roundf(next.y / 32.0f) * 32.0f - now.y;
					player.velocity.x *= 0.75f;
				}
			}
			if (map[x + y * width] == coinID)
			{
				if (now.x > hit.x - 32 &&
					now.x < hit.x + 32 &&
					now.y > hit.y - 32 &&
					now.y < hit.y + 32)
				{
					map[x + y * width] = airID;
					coinSound.Play();
				}
			}
			if (map[x + y * width] == luckyID)
			{
				if (next.x > hit.x - 32 &&
					next.x < hit.x + 32 &&
					now.y > hit.y - 33 &&
					now.y < hit.y + 32)
				{
					player.velocity.x = roundf(next.x / 32.0f) * 32.0f - now.x;
				}

				if (now.x > hit.x - 32 &&
					now.x < hit.x + 32 &&
					next.y > hit.y - 33 &&
					next.y < hit.y + 32)
				{
					if (player.velocity.y > 0.0f)
					{
						map[x + y * width] = blockID;
						coinSound.Play();
					}

					player.isGround = true;
					player.velocity.y = roundf(next.y / 32.0f) * 32.0f - now.y;
					player.velocity.x *= 0.75f;
				}
			}
			if (map[x + y * width] == goalID)
			{
				if (now.x > hit.x - 32 &&
					now.x < hit.x + 32 &&
					now.y > hit.y - 32 &&
					now.y < hit.y + 32)
				{
					clearSound.Play();
					mode = Clear;
				}
			}
		}
	}

	for (int i = 0; i < enemys.size(); i++)
	{
		if (enemys[i].scale.y < 1.0f)
			continue;

		if (player.position.x > enemys[i].position.x - 32 &&
			player.position.x < enemys[i].position.x + 32 &&
			player.position.y > enemys[i].position.y - 32 &&
			player.position.y < enemys[i].position.y + 32)
		{
			if (player.position.y > enemys[i].position.y)
			{
				player.velocity.y = 5.0f;
				enemys[i].position.y -= 12.0f;
				enemys[i].scale.y = 0.5f;
				enemySound.Play();
			}
			else
			{
				overSound.Play();
				mode = Over;
			}
			return;
		}
	}

	player.position += player.velocity;
}

void UpdateEnemy(Character& enemy, Character& player, vector<char>& map, int width, int height)
{
	static int animationCount = 0;

	if (enemy.scale.y < 1.0f)
		return;

	float speed = 0.5f * enemy.side;

	if (!enemy.isGround)
	{
		speed *= 0.3f;
	}

	enemy.velocity.x += speed;

	enemy.velocity.x *= 0.9f;
	enemy.velocity.y -= 0.5f;

	if (enemy.velocity.y < -15.0f)
		enemy.velocity.y = -15.0f;

	enemy.isGround = false;

	int centerX = (int)roundf(enemy.position.x / 32.0f);
	int centerY = height - (int)roundf(enemy.position.y / 32.0f);

	for (int x = centerX - 1; x <= centerX + 1; x++)
	{
		for (int y = centerY - 1; y <= centerY + 1; y++)
		{
			if (x < 0 || y < 0 || x >= width || y >= height)
				continue;

			Float2 now = enemy.position;
			Float2 next = enemy.position + enemy.velocity;
			Float2 hit = Float2(x * 32.0f, (height - y) * 32.0f);

			if (map[x + y * width] == blockID ||
				map[x + y * width] == luckyID)
			{
				if (next.x > hit.x - 32 &&
					next.x < hit.x + 32 &&
					now.y > hit.y - 32 &&
					now.y < hit.y + 32)
				{
					if (enemy.velocity.x > 0)
					{
						enemy.side = -1;
					}
					if (enemy.velocity.x < 0)
					{
						enemy.side = 1;
					}

					enemy.velocity.x = roundf(next.x / 32.0f) * 32.0f - now.x;
				}

				if (now.x > hit.x - 32 &&
					now.x < hit.x + 32 &&
					next.y > hit.y - 32 &&
					next.y < hit.y + 32)
				{
					enemy.isGround = true;
					enemy.velocity.y = roundf(next.y / 32.0f) * 32.0f - now.y;
					enemy.velocity.x *= 0.75f;
				}
			}
		}
	}

	enemy.position += enemy.velocity;

	animationCount++;
	if (animationCount > 10)
	{
		animationCount = 0;
		enemy.scale.x = -enemy.scale.x;
	}
}

void DrawMap(const Character& player, const vector<char>& map, int width, int height)
{
	static Sprite block(L"block.png");
	static Sprite coin(L"coin.png");
	static Sprite lucky(L"lucky.png");
	static Sprite goal(L"goal.png");

	block.scale = 2.0f;
	coin.scale = 2.0f;
	lucky.scale = 2.0f;
	goal.scale = 2.0f;

	int center = (int)roundf(player.position.x / 32.0f);

	for (int x = center - 11; x <= center + 11; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (x < 0 || x >= width)
				continue;

			Float2 position = Float2(x * 32.0f, (height - y) * 32.0f);

			if (map[x + y * width] == blockID)
			{
				block.position = position;
				block.Draw();
			}
			if (map[x + y * width] == coinID)
			{
				coin.position = position;
				coin.angles.y = App::GetTime() * 180.0f;
				coin.Draw();
			}
			if (map[x + y * width] == luckyID)
			{
				lucky.position = position;
				lucky.Draw();
			}
			if (map[x + y * width] == goalID)
			{
				goal.position = position;
				goal.angles.y = App::GetTime() * 180.0f;
				goal.Draw();
			}
		}
	}
}

void DrawCharacter(Sprite& sprite, const Character& character)
{
	sprite.position = character.position;
	sprite.scale = character.scale;
	sprite.Draw();
}

int MAIN()
{

	Camera camera;
	camera.position.y = 260.0f;
	camera.color = Float4(0.5f, 0.75f, 1.0f, 1.0f);

	Sprite playerSprite(L"player.png");
	playerSprite.scale = 2.0f;

	Character player;

	Sprite enemySprite(L"enemy.png");
	enemySprite.scale = 2.0f;

	vector<Character> enemys;

	vector<char> map;

	int width = 0;
	int height = 0;

	Text titleText(L"アクション", 10.0f);
	titleText.scale = 10.0f;
	titleText.color = Float4(1.0f, 0.0f, 0.0f, 1.0f);

	Text clearText(L"クリア", 10.0f);
	clearText.scale = 10.0f;
	clearText.color = Float4(1.0f, 0.0f, 0.0f, 1.0f);

	Text overText(L"ゲーム\nオーバー", 10.0f);
	overText.scale = 10.0f;
	overText.color = Float4(1.0f, 0.0f, 0.0f, 1.0f);

	while (App::Refresh())
	{
		camera.position.x = player.position.x;
		camera.Update();

		switch (mode)
		{
		case Title:
		{
			if (App::GetKeyDown('Z'))
			{
				player = Character();
				enemys.clear();

				map.clear();

				wifstream file(L"stage1.txt");
				wstring line;

				width = 0;
				height = 0;

				while (getline(file, line))
				{
					width = line.length();
					height++;

					for (int i = 0; i < line.length(); i++)
					{
						map.push_back(line[i]);
					}
				}

				for (int x = 0; x < width; x++)
				{
					for (int y = 0; y < height; y++)
					{
						if (map[x + y * width] == playerID)
						{
							player.position = Float3(x * 32.0f, (height - y) * 32.0f, 0.0f);
						}
						if (map[x + y * width] == enemyID)
						{
							Character enemy;
							enemy.position = Float3(x * 32.0f, (height - y) * 32.0f, 0.0f);
							enemys.push_back(enemy);
						}
					}
				}
				mode = Game;
			}

			titleText.position = camera.position;
			titleText.Draw();
		}
		break;
		case Game:

			for (int i = 0; i < enemys.size(); i++)
			{
				UpdateEnemy(enemys[i], player, map, width, height);
			}

			UpdatePlayer(player, enemys, map, width, height);

			DrawMap(player, map, width, height);

			for (int i = 0; i < enemys.size(); i++)
			{
				DrawCharacter(enemySprite, enemys[i]);
			}

			DrawCharacter(playerSprite, player);

			break;
		case Clear:

			if (App::GetKeyDown('Z'))
			{
				mode = Title;
			}

			DrawMap(player, map, width, height);

			for (int i = 0; i < enemys.size(); i++)
			{
				DrawCharacter(enemySprite, enemys[i]);
			}

			DrawCharacter(playerSprite, player);

			clearText.position = camera.position;
			clearText.Draw();

			break;
		case Over:

			if (App::GetKeyDown('Z'))
			{
				mode = Title;
			}

			DrawMap(player, map, width, height);

			for (int i = 0; i < enemys.size(); i++)
			{
				DrawCharacter(enemySprite, enemys[i]);
			}

			DrawCharacter(playerSprite, player);

			overText.position = camera.position;
			overText.Draw();

			break;
		}
	}

	return 0;
}
