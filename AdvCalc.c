#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// linked list data structure
struct variables {
    char * name;
    long long value;
    struct variables * next;
};
struct variables * VARIABLES_HEAD = NULL;

void run();
char * trim(char * );
long long findNofTokens(char *);
char ** tokenize(char *,long long);

// Grammar of the AdvCalc language is explained below in BNF notation

// <assignment> -> <identifier> "=" <expression>
long long isAssignment(char** tokens, long long nofTokens);
long long assignment(char** tokens, long long nofTokens);
// <expression> -> <bitwiseAnd> "|" <expression> | <bitwiseAnd>
long long isExpression(char** tokens, long long nofTokens);
long long evalExpression(char** tokens, long long nofTokens);
// <bitwiseAnd> -> <summation> "&" <bitwiseAnd> | <summation>
long long evalBitwiseAnd(char** tokens, long long nofTokens);
long long isBitwiseAnd(char** tokens, long long nofTokens);
// <summation> -> <multiplication> "+" <summation> | <multiplication> "-" <summation> | <multiplication>
long long isSummation(char** tokens, long long nofTokens);
long long evalSummation(char** tokens, long long nofTokens);
// <multiplication> -> <term> "*" <evalMultiplication> | <term>
long long isMultiplication(char** tokens, long long nofTokens);
long long evalMultiplication(char** tokens, long long nofTokens);
// <term> -> "(" <expression> ")" | <factor>
long long isTerm(char** tokens, long long nofTokens);
long long evalTerm(char** tokens, long long nofTokens);
// <factor> -> <function> | <integer> | <identifier>
long long isFactor(char** tokens, long long nofTokens);
long long evalFactor(char** tokens, long long nofTokens);
// <function> ->       "not" "("  <expression>  ")" | "xor"  "("  <expression>   ","  <expression>   ")" |
// "ls"  "("  <expression>   ","  <expression>   ")" | "rs"  "("  <expression>   ","  <expression>   ")" |
// "lr"  "("  <expression>   ","  <expression>   ")" | "rr"  "("  <expression>   ","  <expression>   ")"
long long isFunction(char** tokens, long long nofTokens);
long long evalFunction(char** tokens, long long nofTokens);
// <identifier> -> <identifier> <alpha> | <alpha>
// <alpha> -> [a-z,A-Z]
long long isIdentifier(char* token);
long long identifier(char* token);
// <integer> ->  <integer> <digit> | <digit>
// <digit> -> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
long long isInteger(char* token);
long long integer(char* token);

long long leftRotation(long long base, long long rotation);
long long rightRotation(long long base, long long rotation);


int main() {
    run();
    return 0;
}

void run() {
    char * string = malloc(1024 * sizeof(char));
    printf("> ");
    if ((fgets(string, 1024, stdin) == NULL)&&feof(stdin)){
        // ctrl-d executed
        printf("\n");
        return;
    }
    string = trim(string);
    // if the input is empty
    if (strlen(string) == 0) {
        free(string);
        run();
        return;
    }

    // returns the number of tokens given input has.
    long long nofTokens = findNofTokens(string);

    // lexical analyzing
    char ** tokens = tokenize(string, nofTokens);

    // parsing & interpreting
    if(isAssignment(tokens,nofTokens))
        assignment(tokens,nofTokens);
    else if(isExpression(tokens,nofTokens))
        printf("%lld\n", evalExpression(tokens, nofTokens));
    else
        printf("Error!\n");

    free(string);
    free(tokens);
    run();
}

// reduces adjacent spaces to single spaces and removes comments from the string.
char * trim(char * string) {
    long long counter = 0, i = 0, flag = 0;
    // iterate over the string and count characters
    while (string[i] != '\0' && string[i] != '%') {
        if(string[i] == '\n') {
            break;
        }
        if (string[i] != ' ') {
            flag = 1;
            counter++;
        }
        else {
            // if previous character was non-whitespace
            if (flag) {
                counter++;
            }
            flag = 0;
        }
        i++;
    }
    char * newString = malloc(++counter * sizeof(char));
    counter = 0;
    i = 0;
    flag = 0;
    // iterates over the string
    while (string[i] != '\0' && string[i] != '\n' && string[i] != '%') {
        // end of the string
        if(string[i] == '\n' || string[i] == '%') {
            i++;
            break;
        }
        if (string[i] != ' ') {
            newString[counter++] = string[i];
            flag = 1;
        }
        else {
            // if previous character was non-whitespace
            if (flag) {
                newString[counter++] = string[i];
            }
            flag = 0;
        }
        i++;
    }
    newString[counter] = '\0';
    free(string);
    return newString;
}

// returns number of tokens given string has
// It works as it counts the number of delimiters such as -, +, /, *, ",".
long long findNofTokens(char * string) {
    long long counter = 0;
    long long i;
    int flag = 0;
    for (i = 0; i < strlen(string); i++) {
        char ch = string[i];
        // delimiters
        if (ch == '+' || ch == '*' || ch == '-' || ch == '&' || ch == '|' || ch == '=' ||  ch == ',' || ch == '(' || ch == ')') {
            counter++;
            if (flag) {
                counter++;
            }
            flag = 0;
        }
        // whitespaces
        else if(ch == ' ' || ch == '\t' || ch == '\n'){
            if (flag) {
                counter++;
            }
            flag = 0;
        }
        // comment indicator
        else if(ch == '%'){
            break;
        }
        else {
            flag = 1;
        }
    }
    if (flag) {
        counter++;
    }
    return counter;
}

// creates an array of strings using the number of tokens given from arguments
// separates the input string into individual tokens, and assign each token to its position in the array.
// then returns the array
char ** tokenize(char * string, long long nofTokens) {
    long long i, j, lastStart = 0, flag = 0, counter = 0;
    // allocating memory
    char ** tokens = malloc(nofTokens * sizeof(char *));
    for (i = 0; i < strlen(string); i++) {
        char ch = string[i];
        // delimiters
        if (ch == '+' || ch == '*' || ch == '-' || ch == '&' || ch == '|' || ch == ',' || ch == '=' || ch == '(' || ch == ')') {
            // if we were previously building up a token, add it to the array
            if (flag) {
                flag = 0;
                tokens[counter] = malloc((i - lastStart + 1) * sizeof(char));
                for (j = lastStart; j < i; j++) {
                    tokens[counter][j - lastStart] = string[j];
                }
                tokens[counter][i - lastStart] = '\0';
                counter++;
            }
            // add the delimiter character as a separate token
            tokens[counter] = malloc((2) * sizeof(char));
            tokens[counter][0] = ch;
            tokens[counter][1] = '\0';
            counter++;
        }
        // after '%' is comment
        else if(ch == '%'){
            // if we were previously building up a token, add it to the array
            if (flag) {
                flag = 0;
                tokens[counter] = malloc((i - lastStart + 1) * sizeof(char));
                for (j = lastStart; j < i; j++) {
                    tokens[counter][j - lastStart] = string[j];
                }
                tokens[counter][i - lastStart] = '\0';
                counter++;
            }
            return tokens;
        }
        else if(ch == ' ' || ch == '\t' || ch == '\n'){
            // if we were previously building up a token, add it to the array
            if (flag) {
                flag = 0;
                tokens[counter] = malloc((i - lastStart + 1) * sizeof(char));
                for (j = lastStart; j < i; j++) {
                    tokens[counter][(j - lastStart)] = string[j];
                }
                tokens[counter][i - lastStart] = '\0';
                counter++;
            }
        }
        else {
            if (!flag) {
                lastStart = i;
            }
            flag = 1;
        }
    }
    // if we were previously building up a token, add it to the array
    if (flag) {
        tokens[counter] = malloc((i - lastStart) * sizeof(char));
        for (j = lastStart; j < i; j++) {
            tokens[counter][j - lastStart] = string[j];
        }
        tokens[counter][i - lastStart] = '\0';
    }
    tokens[counter+1] = NULL;
    return tokens;
}

// <assignment> -> <identifier> "=" <expression>
long long isAssignment(char** tokens, long long nofTokens){
    return ( (nofTokens>=3) && (isIdentifier(tokens[0])) && (!strcmp(tokens[1],"=")) && (isExpression(tokens+2,nofTokens-2)) );
}

// parses the given tokens and assigns the result of expression to the identifier.
long long assignment(char** tokens, long long nofTokens){
    long long res = evalExpression(tokens + 2, nofTokens - 2);
    struct variables * temp = VARIABLES_HEAD;
    // if linked list is empty, it sets given identifier to head of the list
    if (temp == NULL) {
        temp = (struct variables *)malloc(sizeof(struct variables *));
        temp->name = (char *)malloc(sizeof(char));
        temp->name = tokens[0];
        temp->value = res;
        temp->next = NULL;
        VARIABLES_HEAD = temp;
    }
    // if linked list is not empty,
    else {
        // it iterates over the linked list until either the end of the list
        // or finding that given identifier has already in the list.
        while (temp->next != NULL && strcmp(temp->name, tokens[0]) != 0) {
            temp = temp->next;
        }
        // if the identifier in the list, then it updates the value.
        if(!strcmp(temp->name, tokens[0])) {
            temp->value = res;
        }
        // if not, the identifier is added to the end of the list.
        else {
            temp->next = (struct variables *) malloc(sizeof(struct variables *));
            temp = temp->next;
            temp->name = (char *)malloc(sizeof(char));
            temp->name = tokens[0];
            temp->value = res;
            temp->next = NULL;
        }
    }
}

// <expression> -> <bitwiseAnd> "|" <expression> | <bitwiseAnd>
long long isExpression(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"|")){
            if(isBitwiseAnd(tokens,i)&&isExpression(tokens+i+1,nofTokens-(i+1))){
                return 1;
            }
        }
    }
    return isBitwiseAnd(tokens,nofTokens);
}

long long evalExpression(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"|")){
            if(isBitwiseAnd(tokens,i)&&isExpression(tokens+i+1,nofTokens-(i+1))){
                return evalBitwiseAnd(tokens, i) | evalExpression(tokens + i + 1, nofTokens - (i + 1));
            }
        }
    }
    if(isBitwiseAnd(tokens,nofTokens)){
        return evalBitwiseAnd(tokens, nofTokens);
    }
    printf("evalExpression: Error!\n");
    return 0;
}

// <bitwiseAnd> -> <summation> "&" <bitwiseAnd> | <summation>
long long isBitwiseAnd(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"&")){
            if(isSummation(tokens,i)&&isBitwiseAnd(tokens+i+1,nofTokens-(i+1))){
                return 1;
            }
        }
    }
    return isSummation(tokens,nofTokens);
}

long long evalBitwiseAnd(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"&")){
            if(isSummation(tokens,i)&&isBitwiseAnd(tokens+i+1,nofTokens-(i+1))){
                return evalSummation(tokens, i) & evalBitwiseAnd(tokens + i + 1, nofTokens - (i + 1));
            }
        }
    }
    if(isSummation(tokens,nofTokens)){
        return evalSummation(tokens, nofTokens);
    }
    printf("evalBitwiseAnd: Error!\n");
    return 0;
}

// <summation> -> <multiplication> "+" <summation> | <multiplication> "-" <summation> | <multiplication>
long long isSummation(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if((!strcmp(tokens[i],"+"))||(!strcmp(tokens[i],"-"))){
            if(isMultiplication(tokens,i)&&isSummation(tokens+i+1,nofTokens-(i+1))){
                return 1;
            }
        }
    }
    return isMultiplication(tokens,nofTokens);
}

long long evalSummation(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"+")){
            if(isMultiplication(tokens,i)&&isSummation(tokens+i+1,nofTokens-(i+1))){
                return evalMultiplication(tokens, i) + evalSummation(tokens + i + 1, nofTokens - (i + 1));
            }
        }
        else if(!strcmp(tokens[i],"-")){
            if(isMultiplication(tokens,i)&&isSummation(tokens+i+1,nofTokens-(i+1))){
                return evalMultiplication(tokens, i) - evalSummation(tokens + i + 1, nofTokens - (i + 1));
            }
        }
    }
    if(isMultiplication(tokens,nofTokens)){
        return evalMultiplication(tokens, nofTokens);
    }
    printf("evalSummation: Error!\n");
    return 0;
}

// <multiplication> -> <term> "*" <evalMultiplication> | <term>
long long isMultiplication(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"*")){
            if(isTerm(tokens,i)&&isMultiplication(tokens+i+1,nofTokens-(i+1))){
                return 1;
            }
        }
    }
    return isTerm(tokens,nofTokens);
}

long long evalMultiplication(char** tokens, long long nofTokens){
    for(int i = 0; i<nofTokens;i++){
        if(!strcmp(tokens[i],"*")){
            if(isTerm(tokens,i)&&isMultiplication(tokens+i+1,nofTokens-(i+1))){
                return evalTerm(tokens, i) * evalMultiplication(tokens + i + 1, nofTokens - (i + 1));
            }
        }
    }
    if(isTerm(tokens,nofTokens)){
        return evalTerm(tokens, nofTokens);
    }
    printf("evalMultiplication: Error!\n");
    return 0;
}

// <term> -> "(" <expression> ")" | <factor>
long long isTerm(char** tokens, long long nofTokens){
    if((!strcmp(tokens[0],"("))&&(!strcmp(tokens[nofTokens-1],")"))){
        if(isExpression(tokens+1,nofTokens-2)){
            return 1;
        }
    }
    else{
        return isFactor(tokens, nofTokens);
    }
}

long long evalTerm(char** tokens, long long nofTokens){
    if((!strcmp(tokens[0],"("))&&(!strcmp(tokens[nofTokens-1],")"))){
        if(isExpression(tokens+1,nofTokens-2)){
            return evalExpression(tokens + 1, nofTokens - 2);
        }
    }
    if (isFactor(tokens, nofTokens)){
        return evalFactor(tokens, nofTokens);
    }
    printf("evalTerm: Error!");
    return 0;
}

// <factor> -> <function> | <integer> | <identifier>
long long isFactor(char** tokens, long long nofTokens){
    if(nofTokens == 1){
        if(isInteger(tokens[0])){
            return 1;
        }
        if(isIdentifier(tokens[0])){
            return 1;
        }
        return 0;
    }
    return isFunction(tokens,nofTokens);
}

long long evalFactor(char** tokens, long long nofTokens){
    if(nofTokens == 1){
        if(isInteger(tokens[0])){
            return integer(tokens[0]);
        }
        if(isIdentifier(tokens[0])){
            return identifier(tokens[0]);
        }
        printf("evalFactor: Error!");
        return 0;
    }
    else if(isFunction(tokens, nofTokens)){
        return evalFunction(tokens, nofTokens);
    }
    printf("evalFactor: Error!");
    return 0;
}

// <function> ->       "not" "("  <expression>  ")" | "xor"  "("  <expression>   ","  <expression>   ")" |
// "ls"  "("  <expression>   ","  <expression>   ")" | "rs"  "("  <expression>   ","  <expression>   ")" |
// "lr"  "("  <expression>   ","  <expression>   ")" | "rr"  "("  <expression>   ","  <expression>   ")"
long long isFunction(char** tokens, long long nofTokens){
    // one argument function type (not)
    if((!strcmp(tokens[0],"not"))&&(!strcmp(tokens[1],"("))&&(!strcmp(tokens[nofTokens-1],")"))){
        if(isExpression(tokens+2,nofTokens-3)){
            return 1;
        }
    }
    // two argument function types (xor, lr, ls, rs, rr)
    else if((!strcmp(tokens[0],"xor"))||(!strcmp(tokens[0],"ls"))||(!strcmp(tokens[0],"rs"))||(!strcmp(tokens[0],"rr"))||(!strcmp(tokens[0],"lr"))){
        if((nofTokens>=6)&&(!strcmp(tokens[1],"("))&&(!strcmp(tokens[nofTokens-1],")"))) {
            for (int i = 3; i < nofTokens; i++) {
                if (!strcmp(tokens[i], ",")) {
                    if (isExpression(tokens + 2, i - 2) && isExpression(tokens + i + 1, nofTokens - (i + 2))) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

long long evalFunction(char** tokens, long long nofTokens) {
    if((!strcmp(tokens[0],"not"))){
        return ~(evalExpression(tokens + 2, nofTokens - 3));
    }
    else if((!strcmp(tokens[0],"xor"))||(!strcmp(tokens[0],"ls"))||(!strcmp(tokens[0],"rs"))||(!strcmp(tokens[0],"rr"))||(!strcmp(tokens[0],"lr"))) {
        for (int i = 3; i < nofTokens; i++) {
            if (!strcmp(tokens[i], ",")) {
                if (isExpression(tokens + 2, i - 2) && isExpression(tokens + i + 1, nofTokens - (i + 2))) {
                    if (!strcmp(tokens[0], "xor"))
                        return evalExpression(tokens + 2, i - 2) ^ evalExpression(tokens + i + 1, nofTokens - (i + 2));
                    else if (!strcmp(tokens[0], "ls"))
                        return evalExpression(tokens + 2, i - 2) << evalExpression(tokens + i + 1, nofTokens - (i + 2));
                    else if (!strcmp(tokens[0], "rs"))
                        return evalExpression(tokens + 2, i - 2) >> evalExpression(tokens + i + 1, nofTokens - (i + 2));
                    else if (!strcmp(tokens[0], "lr"))
                        return leftRotation(evalExpression(tokens + 2, i - 2),
                                            evalExpression(tokens + i + 1, nofTokens - (i + 2)));
                    else if (!strcmp(tokens[0], "rr"))
                        return rightRotation(evalExpression(tokens + 2, i - 2),
                                             evalExpression(tokens + i + 1, nofTokens - (i + 2)));
                }
            }
        }
    }
    printf("evalFunction: Error!");
    return 0;
}

// <integer> ->  <integer> <digit> | <digit>
long long isInteger(char *token){
    int isnumber = 1 ;
    char *q ;

    for(q = token ; *q != '\0' ; q++) {
        isnumber = isnumber && isdigit(*q) ;
    }

    return(isnumber) ;
}

// converts given string to long long
long long integer(char* token){
    return atoll(token);
}

// <identifier> -> <identifier> <alpha> | <alpha>
long long isIdentifier(char *token){
    // reserved keywords
    if((!strcmp(token,"xor"))||(!strcmp(token,"ls"))||(!strcmp(token,"rs"))||(!strcmp(token,"rr"))||(!strcmp(token,"lr"))||(!strcmp(token,"not"))) {
        return 0;
    }
    int isWord = 1 ;
    char *q ;

    for(q = token ; *q != '\0' ; q++) {
        isWord = isWord && isalpha(*q) ;
    }

    return(isWord) ;
}

// returns the value of the given identifier if it is set previously.
// returns 0 if the given identifier is not set by any value.
long long identifier(char* token){
    struct variables * temp = VARIABLES_HEAD;
    // if the linked list which contains identifiers is empty
    if (temp == NULL) {
        return 0;
    }
    // if the linked list which contains identifiers is not empty
    else {
        // it iterates over the linked list until either the end of the list
        // or finding that given identifier has already in the list.
        while (temp->next != NULL && strcmp(temp->name, token) != 0) {
            temp = temp->next;
        }
        // if the identifier in the list, then it returns the value of the identifier.
        if(!strcmp(temp->name, token)) {
            return temp->value;
        }
        // if not, it returns 0.
        else {
            return 0;
        }
    }
}

// lr(base, rotation)
long long leftRotation(long long base, long long rotation) {
    rotation %= 64;
    long long temp = base;
    temp = temp << rotation;
    base = base >> (64 - rotation);
    return base + temp;
}

// rr(base, rotation)
long long rightRotation(long long base, long long rotation) {
    rotation %= 64;
    long long temp = base;
    temp = temp << (64 - rotation);
    base = base >> rotation;
    return base + temp;
}
