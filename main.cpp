#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>

using namespace std;

// ---------------- Constants ----------------
const int MAX_QUESTION = 1000;
const int MAX_SESSION = 10;
const int BASE_SCORE = 10;       // base points per correct answer
const int MAX_WRONG = 100;

// ---------------- Global Data (parallel arrays) ----------------
int question_Difficulty[MAX_QUESTION];
string question_Text[MAX_QUESTION];
string question_OptionA[MAX_QUESTION];
string question_OptionB[MAX_QUESTION];
string question_OptionC[MAX_QUESTION];
string question_OptionD[MAX_QUESTION];
char question_Correct[MAX_QUESTION];

int total_Loaded = 0;

int filteredIndices[MAX_QUESTION];
int filteredCount = 0;

int selectedIndices[MAX_SESSION];
bool usedFiltered[MAX_QUESTION]; // to avoid reusing same question for swap

// Lifeline flags (one use per quiz)
bool lifeline5050Used = false;
bool lifelineSkipUsed = false;
bool lifelineSwapUsed = false;
bool lifelineTimeUsed = false;

// Review data
int wrongIndex[MAX_WRONG];
char wrongAnswer[MAX_WRONG];
int wrongCount = 0;


// ---------------- Prototypes ----------------
int loadQuestions(const string& filename);
int getCategory(string& outFile);
int getDifficulty();
void filterQuestions(int difficulty);
void shuffleIndices(int arr[], int n);
void startQuiz();
void askQuestion(int index, int& score, int& streak);
void showLeaderboard();
void reviewIncorrect();
void pressEnterToContinue();

int loadQuestions(const string& filename) {
    ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return 0;
    }

    string line;
    total_Loaded = 0;

    while (!fin.eof() && total_Loaded < MAX_QUESTION) {
        // Read difficulty line; skip blank lines
        getline(fin, line);
        if (!fin.good()) break;
        if (line == "") continue;

        question_Difficulty[total_Loaded] = atoi(line.c_str());

        // Next 6 lines expected
        if (!getline(fin, question_Text[total_Loaded])) break;
        if (!getline(fin, question_OptionA[total_Loaded])) break;
        if (!getline(fin, question_OptionB[total_Loaded])) break;
        if (!getline(fin, question_OptionC[total_Loaded])) break;
        if (!getline(fin, question_OptionD[total_Loaded])) break;

        if (!getline(fin, line)) break;
        char c = 'A';
        if (line.size() > 0) c = line[0];
        if (isdigit(static_cast<unsigned char>(c))) 
        c = static_cast<char>('A' + (c - '1'));
        question_Correct[total_Loaded] = (char)toupper(static_cast<unsigned char>(c));

        total_Loaded++;
    }

    fin.close();
    return total_Loaded;
}

// Category menu returns numeric choice and sets outFile to filename
int getCategory(string& outFile) {
    int choice = 0;
    while (choice < 1 || choice > 5) {
        cout << endl << "--- Select Category ---" << endl;
        cout << "1. Science" << endl;
        cout << "2. Computer Science" << endl;
        cout << "3. History" << endl;
        cout << "4. Sports" << endl;
        cout << "5. IQ/Logic" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        if (choice < 1 || choice > 5) cout << "Invalid choice. Please enter 1-5." << endl;
    }

    if (choice == 1) outFile = "science.txt";
    if (choice == 2) outFile = "computer.txt";
    if (choice == 3) outFile = "history.txt";
    if (choice == 4) outFile = "sports.txt";
    if (choice == 5) outFile = "iq.txt";

    return choice;
}

int getDifficulty() {
    int diff = 0;
    while (diff < 1 || diff > 3) {
        cout << endl << "--- Select Difficulty ---" << endl;
        cout << "1. Easy" << endl;
        cout << "2. Medium" << endl;
        cout << "3. Hard" << endl;
        cout << "Enter difficulty: ";
        cin >> diff;
        if (diff < 1 || diff > 3) cout << "Invalid input. Please enter 1,2 or 3." << endl;
    }
    return diff;
}

// Build filteredIndices[] containing indices of loaded questions matching difficulty
void filterQuestions(int difficulty) {
    filteredCount = 0;
    for (int i = 0; i < total_Loaded; i++) {
        if (question_Difficulty[i] == difficulty) {
            filteredIndices[filteredCount] = i;
            filteredCount++;
        }
    }
}

// Simple Fisher-Yates-like shuffle for integer array
void shuffleIndices(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        int r = rand() % n;
        int tmp = arr[i];
        arr[i] = arr[r];
        arr[r] = tmp;
    }
}

// Press enter helper
void pressEnterToContinue() {
    cin.clear();
    cin.ignore(1000, '\n');
    cout << "Press Enter to continue...";
    cin.get();
}
void askQuestion(int index, int& score, int& streak) {
    // local copies for display so 50/50 doesn't permanently modify global options
    string optA = question_OptionA[index];
    string optB = question_OptionB[index];
    string optC = question_OptionC[index];
    string optD = question_OptionD[index];

    bool questionAnswered = false;

    while (!questionAnswered) {
        cout << endl << "---------------------------------" << endl;
        cout << question_Text[index] << endl;
        cout << "A) " << optA << endl;
        cout << "B) " << optB << endl;
        cout << "C) " << optC << endl;
        cout << "D) " << optD << endl;
        cout << "Enter your answer (A/B/C/D or 1-4) or press U to use a lifeline: ";

        string input;
        time_t start = time(0);
        cin >> input;
        time_t end = time(0);
        double elapsed = difftime(end, start);

        // If user chose lifeline entry
        if (input.size() > 0 && (input[0] == 'U' || input[0] == 'u')) {
            cout << endl << "--- Lifeline Menu ---" << endl;
            if (!lifeline5050Used) cout << "1. 50/50 (remove 2 wrong options)" << endl;
            if (!lifelineSkipUsed) cout << "2. Skip question (no penalty)" << endl;
            if (!lifelineSwapUsed) cout << "3. Swap question (random unused)" << endl;
            if (!lifelineTimeUsed) cout << "4. Extra Time (+10s for this question)" << endl;
            cout << "0. Cancel" << endl;
            cout << "Enter choice: ";
            int lfChoice;
            cin >> lfChoice;

            if (lfChoice == 1 && !lifeline5050Used) {
                lifeline5050Used = true;
                // hide two incorrect options in local copies
                char corr = question_Correct[index];
                // find two wrong indices to hide
                int hide1 = -1, hide2 = -1;
                while (true) {
                    int r = rand() % 4;
                    if (r == 0 && corr != 'A') { hide1 = 0; break; }
                    if (r == 1 && corr != 'B') { hide1 = 1; break; }
                    if (r == 2 && corr != 'C') { hide1 = 2; break; }
                    if (r == 3 && corr != 'D') { hide1 = 3; break; }
                }
                while (true) {
                    int r = rand() % 4;
                    if (r != hide1) {
                        if (r == 0 && corr != 'A') { hide2 = 0; break; }
                        if (r == 1 && corr != 'B') { hide2 = 1; break; }
                        if (r == 2 && corr != 'C') { hide2 = 2; break; }
                        if (r == 3 && corr != 'D') { hide2 = 3; break; }
                    }
                }
                if (hide1 == 0 || hide2 == 0) optA = "(removed)";
                if (hide1 == 1 || hide2 == 1) optB = "(removed)";
                if (hide1 == 2 || hide2 == 2) optC = "(removed)";
                if (hide1 == 3 || hide2 == 3) optD = "(removed)";
                cout << "50/50 applied. Two incorrect options removed." << endl;
                continue; // re-display question with removed options
            }
            else if (lfChoice == 2 && !lifelineSkipUsed) {
                lifelineSkipUsed = true;
                cout << "Skip used — question skipped (no penalty)." << endl;
                return; // skip this question
            }
            else if (lfChoice == 3 && !lifelineSwapUsed) {
                lifelineSwapUsed = true;
                // find a random unused filtered question
                int newIndex = -1;
                int attempts = 0;
                while (attempts < filteredCount) {
                    int r = rand() % filteredCount;
                    int candidate = filteredIndices[r];
                    if (!usedFiltered[candidate]) {
                        newIndex = candidate;
                        break;
                    }
                    attempts++;
                }
                if (newIndex == -1) {
                    cout << "Swap unavailable: no unused filtered question left." << endl;
                    continue;
                }
                else {
                    usedFiltered[newIndex] = true;
                    index = newIndex;
                    // refresh local copies for new question
                    optA = question_OptionA[index];
                    optB = question_OptionB[index];
                    optC = question_OptionC[index];
                    optD = question_OptionD[index];
                    cout << "Question swapped — new question loaded." << endl;
                    continue; // re-display the new question
                }
            }
            else if (lfChoice == 4 && !lifelineTimeUsed) {
                lifelineTimeUsed = true;
                cout << "Extra time used for this question (+10 seconds)." << endl;
                // Ask immediately with extended allowance (we'll measure elapsed after input)
                cout << "Enter your answer (A/B/C/D or 1-4): ";
                string ansInput;
                time_t s2 = time(0);
                cin >> ansInput;
                time_t e2 = time(0);
                double elapsed2 = difftime(e2, s2);
                bool timeout = (elapsed2 > 20.0); // 10 + 10 extra
                char ansChar = (ansInput.size() > 0) ? toupper(static_cast<unsigned char>(ansInput[0])) : '?';
                if (ansChar == '1') ansChar = 'A';
                if (ansChar == '2') ansChar = 'B';
                if (ansChar == '3') ansChar = 'C';
                if (ansChar == '4') ansChar = 'D';
                if (ansChar < 'A' || ansChar > 'D') ansChar = '?';

                if (timeout) {
                    cout << "Time exceeded. Marked as unanswered/wrong." << endl;
                    // penalty logic same as wrong
                    streak = 0;
                    int penalty = 0;
                    if (question_Difficulty[index] == 1) penalty = 2;
                    else if (question_Difficulty[index] == 2) penalty = 3;
                    else penalty = 5;
                    score -= penalty;
                    cout << "Penalty: -" << penalty << endl;
                    if (wrongCount < MAX_WRONG) { wrongIndex[wrongCount] = index; wrongAnswer[wrongCount] = '?'; wrongCount++; }
                    if (score < 0) score = 0;
                    cout << "Current Score: " << score << endl;
                    return;
                }
                else {
                    // evaluate ansChar
                    char corr = question_Correct[index];
                    if (ansChar == corr) {
                        cout << "Correct!" << endl;
                        score += BASE_SCORE;
                        streak++;
                        if (streak == 3) { cout << "STREAK BONUS +5!" << endl; score += 5; }
                        if (streak == 5) { cout << "MEGA STREAK BONUS +15!" << endl; score += 15; }
                    }
                    else {
                        cout << "Incorrect! Correct was: " << corr << endl;
                        streak = 0;
                        int penalty = 0;
                        if (question_Difficulty[index] == 1) penalty = 2;
                        else if (question_Difficulty[index] == 2) penalty = 3;
                        else penalty = 5;
                        score -= penalty;
                        cout << "Penalty: -" << penalty << endl;
                        if (wrongCount < MAX_WRONG) { wrongIndex[wrongCount] = index; wrongAnswer[wrongCount] = ansChar; wrongCount++; }
                        if (score < 0) score = 0;
                    }
                    cout << "Current Score: " << score << endl;
                    return;
                }
            }
            else {
                cout << "Invalid choice or lifeline already used." << endl;
                continue;
            }
        } // end lifeline handling

        // Normal answer path: input is already captured in 'input', elapsed measured
        bool timeoutNormal = false;
        double allowed = 10.0;
        if (lifelineTimeUsed) allowed += 10.0; // if lifelineTimeUsed was toggled earlier in same question, allow it
        if (elapsed > allowed) timeoutNormal = true;

        char answerChar = (input.size() > 0) ? toupper(static_cast<unsigned char>(input[0])) : '?';
        if (answerChar == '1') answerChar = 'A';
        if (answerChar == '2') answerChar = 'B';
        if (answerChar == '3') answerChar = 'C';
        if (answerChar == '4') answerChar = 'D';

        // If user selected an option that was removed by 50/50, treat as invalid/wrong
        if (optA == "(removed)" && answerChar == 'A') {
            cout << "That option was removed by 50/50. Counting as wrong." << endl;
            answerChar = '?';
        }
        if (optB == "(removed)" && answerChar == 'B') {
            cout << "That option was removed by 50/50. Counting as wrong." << endl;
            answerChar = '?';
        }
        if (optC == "(removed)" && answerChar == 'C') {
            cout << "That option was removed by 50/50. Counting as wrong." << endl;
            answerChar = '?';
        }
        if (optD == "(removed)" && answerChar == 'D') {
            cout << "That option was removed by 50/50. Counting as wrong." << endl;
            answerChar = '?';
        }

        if (timeoutNormal) {
            cout << "\nTime exceeded (" << elapsed << "s). Marked as unanswered / wrong." << endl;
            // penalty by difficulty
            streak = 0;
            int penalty = 0;
            if (question_Difficulty[index] == 1) penalty = 2;
            else if (question_Difficulty[index] == 2) penalty = 3;
            else penalty = 5;
            score -= penalty;
            cout << "Penalty: -" << penalty << endl;
            if (wrongCount < MAX_WRONG) { wrongIndex[wrongCount] = index; wrongAnswer[wrongCount] = '?'; wrongCount++; }
            if (score < 0) score = 0;
            cout << "Current Score: " << score << endl;
            return;
        }

        // Validate answerChar
        if (!(answerChar >= 'A' && answerChar <= 'D')) {
            cout << "Invalid input — counted as wrong." << endl;
            answerChar = '?';
        }

        char correct = question_Correct[index];

        if (answerChar == correct) {
            cout << "Correct!" << endl;
            score += BASE_SCORE;
            streak++;
            if (streak == 3) { cout << "STREAK BONUS +5!" << endl; score += 5; }
            if (streak == 5) { cout << "MEGA STREAK BONUS +15!" << endl; score += 15; }
        }
        else {
            cout << "Incorrect! Correct answer: " << correct << endl;
            streak = 0;
            int penalty = 0;
            if (question_Difficulty[index] == 1) penalty = 2;
            else if (question_Difficulty[index] == 2) penalty = 3;
            else penalty = 5;
            score -= penalty;
            cout << "Penalty: -" << penalty << endl;
            if (wrongCount < MAX_WRONG) { wrongIndex[wrongCount] = index; wrongAnswer[wrongCount] = answerChar; wrongCount++; }
            if (score < 0) score = 0;
        }

        cout << "Current Score: " << score << endl;
        questionAnswered = true;
    } // end while not answered
}

// Start the quiz (main flow)
void startQuiz() {
    // reset lifelines and usedFiltered and wrongs
    lifeline5050Used = lifelineSkipUsed = lifelineSwapUsed = lifelineTimeUsed = false;
    wrongCount = 0;
    for (int i = 0; i < MAX_QUESTION; i++) usedFiltered[i] = false;

    string categoryFile;
    int cat = getCategory(categoryFile);
    int diff = getDifficulty();

    cout << endl << "Loading questions from: " << categoryFile << endl;
    int loaded = loadQuestions(categoryFile);
    cout << "Questions loaded: " << loaded << endl;
    if (loaded == 0) {
        cout << "No questions found or file missing." << endl;
        return;
    }

    filterQuestions(diff);
    if (filteredCount < MAX_SESSION) {
        cout << "Not enough questions to start quiz. Need at least " << MAX_SESSION << " matching questions." << endl;
        return;
    }

    // Shuffle filtered indices and pick first MAX_SESSION
    shuffleIndices(filteredIndices, filteredCount);
    for (int i = 0; i < MAX_SESSION; i++) {
        selectedIndices[i] = filteredIndices[i];
        usedFiltered[selectedIndices[i]] = true;
    }

    cout << endl << "Quiz is ready! Starting now..." << endl;

    int score = 0;
    int streak = 0;

    for (int i = 0; i < MAX_SESSION; i++) {
        askQuestion(selectedIndices[i], score, streak);
    }

    cout << endl << "==== QUIZ FINISHED ====" << endl;
    cout << "Final Score: " << score << endl;

    // Review wrongs prompt
    char rev;
    cout << "Do you want to review incorrect questions? (Y/N): ";
    cin >> rev;
    if (toupper(static_cast<unsigned char>(rev)) == 'Y') reviewIncorrect();

    // Save leaderboard (append)
    cin.clear();
    cin.ignore(1000, '\n');
    string playerName;
    cout << "Enter your name for leaderboard: ";
    getline(cin, playerName);
    if (playerName == "") playerName = "Anonymous";

    ofstream fout("high_scores.txt", ios::app);
    if (fout.is_open()) {
        // use pipe-separated fields: name|score|difficulty
        fout << playerName << "|" << score << "|" << diff << endl;
        fout.close();
        cout << "Saved to leaderboard." << endl;
    }
    else {
        cout << "Failed to open leaderboard file for writing." << endl;
    }

    pressEnterToContinue();
}

// Read leaderboard, sort descending by score, display top 5
void showLeaderboard() {
    ifstream fin("high_scores.txt");
    if (!fin.is_open()) {
        cout << "No leaderboard data found." << endl;
        return;
    }

    const int MAX_LEAD = 500;
    string names[MAX_LEAD];
    int scores[MAX_LEAD];
    int diffs[MAX_LEAD];
    int count = 0;

    string line;
    while (getline(fin, line) && count < MAX_LEAD) {
        if (line == "") continue;
        int p1 = (int)line.find('|');
        int p2 = (int)line.find('|', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        names[count] = line.substr(0, p1);
        scores[count] = atoi(line.substr(p1 + 1, p2 - p1 - 1).c_str());
        diffs[count] = atoi(line.substr(p2 + 1).c_str());
        count++;
    }
    fin.close();

    if (count == 0) {
        cout << "Leaderboard is empty." << endl;
        return;
    }

    // simple bubble sort descending by score (small N)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j] < scores[j + 1]) {
                int ts = scores[j]; scores[j] = scores[j + 1]; scores[j + 1] = ts;
                string tn = names[j]; names[j] = names[j + 1]; names[j + 1] = tn;
                int td = diffs[j]; diffs[j] = diffs[j + 1]; diffs[j + 1] = td;
            }
        }
    }

    cout << endl << "======= LEADERBOARD (Top 5) =======" << endl;
    int limit = (count < 5) ? count : 5;
    for (int i = 0; i < limit; i++) {
        cout << (i + 1) << ". " << names[i] << " | Score: " << scores[i] << " | Difficulty: " << diffs[i] << endl;
    }
    pressEnterToContinue();
}

// Review incorrect answers
void reviewIncorrect() {
    if (wrongCount == 0) {
        cout << "No incorrect answers to review. Good job!" << endl;
        return;
    }
    cout << endl << "===== REVIEW INCORRECT (" << wrongCount << ") =====" << endl;
    for (int i = 0; i < wrongCount; i++) {
        int qidx = wrongIndex[i];
        char ua = wrongAnswer[i];
        char ca = question_Correct[qidx];
        cout << endl << (i + 1) << ") " << question_Text[qidx] << endl;
        cout << "A) " << question_OptionA[qidx] << endl;
        cout << "B) " << question_OptionB[qidx] << endl;
        cout << "C) " << question_OptionC[qidx] << endl;
        cout << "D) " << question_OptionD[qidx] << endl;
        string utext = (ua == 'A') ? ("A) " + question_OptionA[qidx]) :
            (ua == 'B') ? ("B) " + question_OptionB[qidx]) :
            (ua == 'C') ? ("C) " + question_OptionC[qidx]) :
            (ua == 'D') ? ("D) " + question_OptionD[qidx]) :
            "No answer / invalid";
        string ctext = (ca == 'A') ? ("A) " + question_OptionA[qidx]) :
            (ca == 'B') ? ("B) " + question_OptionB[qidx]) :
            (ca == 'C') ? ("C) " + question_OptionC[qidx]) :
            (ca == 'D') ? ("D) " + question_OptionD[qidx]) :
            "Unknown";
        cout << "Your answer : " << utext << endl;
        cout << "Correct     : " << ctext << endl;
        cout << "---------------------------------" << endl;
    }
    pressEnterToContinue();
}
// ---------------- Main ----------------
int main() {
    srand((unsigned int)time(0));

    int choice = 0;
    do {
        cout << endl << "===== CONSOLE QUIZ GAME =====" << endl;
        cout << "1. Start Quiz" << endl;
        cout << "2. View Leaderboard" << endl;
        cout << "3. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) startQuiz();
        else if (choice == 2) showLeaderboard();
        else if (choice == 3) cout << "Exiting program. Good luck!" << endl;
        else cout << "Invalid choice. Try again." << endl;
    } while (choice != 3);
    system("pause");
    return 0;
}
