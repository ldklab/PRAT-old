/*  C/C++ program to remove invalid parenthesis */
#include<string>
#include<queue>
#include<set>
#include<iostream>

using namespace std;

// Method checks if character is parenthesis (open 
// or closed).
bool isParenthesis(char c) {
    return ((c == '(') || (c == ')'));
}

// Method returns true if string contains valid 
// parenthesis.
bool isValidString(string str) {
    int cnt = 0;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '(')
            cnt++;
        else if (str[i] == ')')
            cnt--;
        if (cnt < 0)
            return false;
    }
    return (cnt == 0);
}

//  method to remove invalid parenthesis 
void removeInvalidParenthesis(string str) {
    if (str.empty())
        return;

    // Visit set to ignore already visited string.
    set <string> visit;

    // Queue to maintain BFS.
    queue <string> q;
    string temp;
    bool level;

    // Pushing given string as starting node onto queue.
    q.push(str);
    visit.insert(str);

    while (!q.empty()) {
        str = q.front();
        q.pop();
        if (isValidString(str)) {
            cout << str << endl;

            // If answer is found, make level true 
            // so that valid string of only that level 
            // is processed. 
            level = true;
        }

        if (level)
            continue;

        for (int i = 0; i < str.length(); i++) {
            if (!isParenthesis(str[i]))
                continue;

            // Removing parenthesis from str and 
            // pushing into queue, if not visited already .
            temp = str.substr(0, i) + str.substr(i + 1);
            if (visit.find(temp) == visit.end()) {
                q.push(temp);
                visit.insert(temp);
            }
        }
    }
}

//  Driver code to check above methods.
int main() {
    string expression = "()())()";
    removeInvalidParenthesis(expression);

    expression = "()v)";
    removeInvalidParenthesis(expression);

    return 0;
} 