# PF-Project-Semester-1-QUIZ-GAME
A fully interactive, category-based quiz game written in C++.
It features multiple categories, three difficulty levels, lifelines, timed questions, score streak bonuses, and a persistent leaderboard.

This project is ideal for learning file handling, arrays, functions, user interaction, and game logic in C++.

🚀 Features
✅ 1. Multiple Categories
Players can choose from:

Science
Computer Science
History
Sports
IQ & Logic

Each category loads questions from its own text file (e.g., science.txt, computer.txt).

🎚 2. Difficulty Levels
Three difficulty modes:
Easy
Medium
Hard

Filtering ensures only questions of the chosen difficulty appear in the quiz.
🔢 3. 10-Question Session
Each quiz session:
Randomly selects 10 unique questions
Shuffles questions each time
Tracks streaks and penalties
🛟 4. Lifelines (One Use Each)

The game includes 4 lifelines:
Lifeline	Function
50/50	Removes 2 incorrect options
Skip	Skip question without penalty
Swap	Replace question with an unused one
Extra Time	+10 seconds for the current question

⏳ 5. Timed Questions

Base time: 10 seconds

With Extra Time lifeline: 20 seconds

Exceeding time counts as a wrong answer with penalty.

🏆 6. Scoring System

Correct answer: +10 points

Streak bonus at:

3 correct in a row → +5 bonus

5 correct in a row → +15 mega bonus

Wrong answer penalty based on difficulty:

Easy → –2

Medium → –3

Hard → –5

Score never drops below 0.

📜 7. Review Incorrect Answers

At the end of the quiz:

Player can review all wrong answers

Correct choices vs. user's choices are displayed clearly

📝 8. Leaderboard

Stores name, score, and difficulty level

Saves results to high_scores.txt

Leaderboard menu shows Top 5 scores, sorted automatically

📂 File Structure
/project-folder
│
├── main.cpp               # Main quiz game source code
├── science.txt            # Category question file
├── computer.txt
├── history.txt
├── sports.txt
├── iq.txt
├── high_scores.txt        # Auto-created leaderboard
└── README.md              # Documentation


Each question file follows this format:

2
What is the boiling point of water?
100°C
90°C
80°C
50°C
1


Format per question:

Difficulty (1–3)

Question text

Option A

Option B

Option C

Option D

Correct option (A-D or 1-4)

🛠️ How to Compile and Run
Using g++:
g++ main.cpp -o quizgame
./quizgame

On Windows (MinGW):
g++ main.cpp -o quizgame.exe
quizgame.exe

📘 How the Program Works
1. Startup Menu
1. Start Quiz
2. View Leaderboard
3. Exit

2. Select Category → Difficulty

Program loads and filters questions.

3. 10 Round Quiz

Displays question, options, timer, lifeline menu, and score updates.

4. End Summary

Final score

Option to review wrong questions

Enter name for leaderboard
📎 Concepts Demonstrated
This project teaches:
File I/O (ifstream, ofstream)
Parallel arrays
Randomization and shuffling
String parsing
Time measurement (time(), difftime())
Input validation
Modular C++ coding with functions
Basic data persistence (leaderboard)
⭐ Future Enhancements (Optional Ideas)
Add sound effects and animations
Add unlimited question bank
Convert into GUI version (SFML / Qt)
Track average performance
Add achievements or badges

🙌 Author

Developed as a console-based interactive quiz system for learning and practice.
