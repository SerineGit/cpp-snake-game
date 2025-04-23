#pragma once



#include <time.h>
#include <stdlib.h>

namespace Snake {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;

	public ref class GameForm : public System::Windows::Forms::Form
	{
	public:
		GameForm(void)
		{
			InitializeComponent();
			this->DoubleBuffered = true;
			StartNewGame();
		}

		ref class HighScore
		{
		public:
			String^ playerName;
			int score;

			HighScore(String^ name, int sc) {
				playerName = name;
				score = sc;
			}
		};

	protected:
		~GameForm()
		{
			if (components)
			{
				delete components;
			}
		}

		// Draw snake and target
		virtual void OnPaint(System::Object^ obj, PaintEventArgs^ e) override {
			Graphics^ g = e->Graphics;

			Brush^ targetBrush = gcnew SolidBrush(Color::HotPink);
			if (playEatEffect) {
				Pen^ effectPen = gcnew Pen(Color::Yellow, 3);
				int size = eatEffectRadius;
				g->DrawEllipse(effectPen,
					eatEffectPosition.X + blockSize / 2 - size / 2,
					eatEffectPosition.Y + blockSize / 2 - size / 2,
					size, size);
			}

			int pulse = (DateTime::Now.Millisecond / 100) % 2 == 0 ? 2 : -2;
			int size = blockSize + pulse;
			g->FillEllipse(targetBrush,
				targetPosition.X + (blockSize - size) / 2,
				targetPosition.Y + (blockSize - size) / 2,
				size, size);

			Brush^ snakeBrush = gcnew SolidBrush(Color::Lime);
			for (int i = 0; i < snakeBody->Count; i++) {
				Color segColor = snakeColors[i];
				Brush^ brush = gcnew SolidBrush(segColor);
				g->FillRectangle(brush, snakeBody[i].X, snakeBody[i].Y, blockSize, blockSize);
			}
		}

	private:
		System::ComponentModel::Container^ components;
		List <Point>^ snakeBody;
		Point targetPosition;
		const int blockSize = 20;
		List<HighScore^>^ highScores = gcnew List<HighScore^>();

		int targetCount = 0;
		Label^ scoreLabel;
		Label^ labelGameOver;
		Label^ labelHighScores;
		Button^ btnNewGame;

		Timer^ timer;
		int moveX = 1, moveY = 0;
		Timer^ gameOverTimer;
		int gameOverBlinkCount = 0;

		bool playEatEffect = false;
		Point eatEffectPosition;
		int eatEffectRadius = 0;
		Timer^ effectTimer;

		List<Color>^ snakeColors;

		void StartNewGame()
		{
			snakeColors = gcnew List<Color>();
			snakeColors->Add(GetRandomColor());

			targetCount = 0;
			scoreLabel->Text = "Score: 0";
			moveX = 1;
			moveY = 0;

			snakeBody->Clear();
			snakeBody->Add(Point(100, 100));

			PlaceTarget();

			labelGameOver->Visible = false;
			btnNewGame->Visible = false;

			labelHighScores->Text = "";

			timer->Start();
			this->Focus();
		}

		void AddHighScore(int score)
		{
			highScores->Add(gcnew HighScore("Player", score));
			highScores->Sort(gcnew Comparison<HighScore^>(this, &GameForm::CompareHighScores));
			if (highScores->Count > 5) {
				highScores->RemoveAt(5);
			}
		}

		int CompareHighScores(HighScore^ hs1, HighScore^ hs2) {
			return hs2->score.CompareTo(hs1->score);
		}

		void ShowHighScores()
		{
			String^ scoresText = "Top 5 High Scores:\n";
			for (int i = 0; i < Math::Min(highScores->Count, 5); i++) {
				scoresText += (i + 1) + ". " + highScores[i]->playerName + " - " + highScores[i]->score + "\n";
			}
			labelHighScores->Text = scoresText;
		}

		void StartGameOverAnimation() {
			gameOverBlinkCount = 0;
			scoreLabel->Visible = true;
			labelGameOver->Visible = true;
			labelGameOver->Text = "Game Over";
			timer->Stop();

			gameOverTimer = gcnew Timer();
			gameOverTimer->Interval = 300;
			gameOverTimer->Tick += gcnew EventHandler(this, &GameForm::OnGameOverBlink);
			gameOverTimer->Start();

			AddHighScore(targetCount);
			ShowHighScores();
			snakeBody->Clear();
			targetPosition = Point(-10, -10);
			this->Invalidate();
		}

		void OnGameOverBlink(Object^ sender, EventArgs^ e) {
			gameOverBlinkCount++;
			labelGameOver->Visible = (gameOverBlinkCount % 2 == 0);
			if (gameOverBlinkCount >= 6) {
				gameOverTimer->Stop();
				labelGameOver->Visible = true;
				btnNewGame->Visible = true;
			}
		}

		void OnKeyDown(Object^ obj, KeyEventArgs^ e) {
			switch (e->KeyCode) {
			case Keys::Up:
				if (moveY != 1) {
					moveX = 0;
					moveY = -1;
				}
				break;
			case Keys::Down:
				if (moveY != -1) {
					moveX = 0;
					moveY = 1;
				}
				break;
			case Keys::Left:
				if (moveX != 1) {
					moveX = -1;
					moveY = 0;
				}
				break;
			case Keys::Right:
				if (moveX != -1) {
					moveX = 1;
					moveY = 0;
				}
				break;
			}
		}

		void OnTimerTick(Object^ obj, EventArgs^ e) {
			MoveSnake();

			if (snakeBody[0].X < 0 || snakeBody[0].Y < 0 || snakeBody[0].X >= this->ClientSize.Width || snakeBody[0].Y >= this->ClientSize.Height)
			{
				timer->Stop();
				StartGameOverAnimation();
				return;
			}

			if (snakeBody->Count >= 4) {
				for (int i = 1; i < snakeBody->Count; i++) {
					if (snakeBody[0] == snakeBody[i]) {
						timer->Stop();
						StartGameOverAnimation();
						return;
					}
				}
			}

			if (snakeBody[0] == targetPosition) {
				targetCount++;
				scoreLabel->Text = "Score: " + targetCount.ToString();
				GrowSnake();
				PlaceTarget();
				playEatEffect = true;
				eatEffectPosition = snakeBody[0];
				eatEffectRadius = 5;

				if (effectTimer == nullptr) {
					effectTimer = gcnew Timer();
					effectTimer->Interval = 30;
					effectTimer->Tick += gcnew EventHandler(this, &GameForm::OnEffectTick);
				}
				effectTimer->Start();
			}

			this->Invalidate();
		}

		void MoveSnake() {
			Point newHead = snakeBody[0];
			newHead.X += moveX * blockSize;
			newHead.Y += moveY * blockSize;
			snakeBody->Insert(0, newHead);
			snakeBody->RemoveAt(snakeBody->Count - 1);
			snakeColors->Insert(0, snakeColors[0]);
			snakeColors->RemoveAt(snakeColors->Count - 1);
		}

		void GrowSnake() {
			Point newHead = snakeBody[0];
			newHead.X += moveX * blockSize;
			newHead.Y += moveY * blockSize;
			snakeBody->Insert(0, newHead);
			snakeColors->Insert(0, GetRandomColor());
		}

		void PlaceTarget() {
			int maxX = this->ClientSize.Width / blockSize;
			int maxY = this->ClientSize.Height / blockSize;
			do {
				targetPosition = Point(rand() % maxX * blockSize, rand() % maxY * blockSize);
			} while (snakeBody->Contains(targetPosition));
		}

		void btnNewGame_Click(Object^ sender, EventArgs^ e)
		{
			StartNewGame();
		}

		void OnEffectTick(Object^ sender, EventArgs^ e) {
			eatEffectRadius += 5;
			if (eatEffectRadius >= 40) {
				effectTimer->Stop();
				playEatEffect = false;
			}
			this->Invalidate();
		}

		Color GetRandomColor() {
			Random^ rand = gcnew Random(Guid::NewGuid().GetHashCode());
			return Color::FromArgb(255, rand->Next(100, 256), rand->Next(100, 256), rand->Next(100, 256));
		}

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->SuspendLayout();
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(500, 550);
			this->Text = L"Snake Game";
			this->BackColor = Color::Black;
			this->StartPosition = FormStartPosition::CenterScreen;
			this->DoubleBuffered = true;
			this->ResumeLayout(false);

			scoreLabel = gcnew Label();
			scoreLabel->ForeColor = Color::Yellow;
			scoreLabel->BackColor = Color::Transparent;
			scoreLabel->Text = "Score: 0";
			scoreLabel->Location = Point(10, 10);
			this->Controls->Add(scoreLabel);

			labelGameOver = gcnew Label();
			labelGameOver->Visible = false;
			labelGameOver->Location = Point(155, 150);
			labelGameOver->Text = "Game Over";
			labelGameOver->Font = gcnew System::Drawing::Font("Press Start 2P", 26, FontStyle::Bold);
			labelGameOver->ForeColor = Color::Orange;
			labelGameOver->BackColor = Color::Transparent;
			labelGameOver->Size = Drawing::Size(210, 75);
			this->Controls->Add(labelGameOver);

			labelHighScores = gcnew Label();
			labelHighScores->Location = Point(10, 300);
			labelHighScores->ForeColor = Color::Cyan;
			labelHighScores->BackColor = Color::Transparent;
			labelHighScores->Size = Drawing::Size(300, 150);
			labelHighScores->Text = "";
			labelHighScores->Font = gcnew System::Drawing::Font("Press Start 2P", 14, FontStyle::Regular);
			this->Controls->Add(labelHighScores);

			btnNewGame = gcnew Button();
			btnNewGame->Visible = false;
			btnNewGame->Text = "New Game";
			btnNewGame->Location = Point(175, 225);
			btnNewGame->BackColor = Color::LimeGreen;
			btnNewGame->FlatStyle = FlatStyle::Flat;
			btnNewGame->FlatAppearance->BorderColor = Color::WhiteSmoke;
			btnNewGame->FlatAppearance->BorderSize = 3;
			btnNewGame->Font = gcnew System::Drawing::Font("Press Start 2P", 14, FontStyle::Bold);
			btnNewGame->ForeColor = Color::WhiteSmoke;
			btnNewGame->Size = Drawing::Size(150, 50);
			btnNewGame->Padding = System::Windows::Forms::Padding(5);
			btnNewGame->TextAlign = ContentAlignment::MiddleCenter;
			btnNewGame->Click += gcnew EventHandler(this, &GameForm::btnNewGame_Click);
			this->Controls->Add(btnNewGame);

			snakeBody = gcnew List <Point>();
			snakeBody->Add(Point(100, 100));

			srand(time(NULL));
			PlaceTarget();

			timer = gcnew Timer();
			timer->Interval = Math::Max(50, 200 - targetCount * 5);
			timer->Tick += gcnew EventHandler(this, &GameForm::OnTimerTick);
			timer->Start();

			this->Paint += gcnew PaintEventHandler(this, &GameForm::OnPaint);
			this->KeyDown += gcnew KeyEventHandler(this, &GameForm::OnKeyDown);
		}
#pragma endregion
	};
}
