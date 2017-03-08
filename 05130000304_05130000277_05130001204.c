#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGHT 255
#define MAX_VAR_LENGHT 32

// 0513-304 Berçem ÇATALKAYA
// 0513-1204 Pınar AKARSU
// 0513-277 Tuğçe DÜLGE

enum token
{
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    STRING,
    OPERATOR,
    PARANTHESIS,
    UNKNOWN

};

enum error
{
    MAX_STRING_LENGTH_ERROR,
    MAX_VAR_LENGTH_ERROR,
    UNKNOWN_SYMBOL_ERROR
};

char token_names[7][20] =
{
    "KEYWORD","IDENTIFIER","NUMBER",
    "STRING","OPERATOR","PARANTHESIS","UNKNOWN"
};
char *error_strings[3] =
{
    "Max string length exceeded",
    "Max variable name length exceeded",
    "Unknown character"
};

char tokenCounts[7] = {0};
int line = 1, column = 0, lastColCount = 0;

void doLexicalAnalysis(FILE *);
int main()
{
    FILE *inputFile;

    if((inputFile = fopen("deneme.for","r")) == NULL)
    {
        printf("File not exist!");
        return 0;
    }

    printf("\n%8s%15s  %s\n\n","POSITION","TOKEN","VALUE");
    doLexicalAnalysis(inputFile); //analize gonder
    printResult();
    fclose(inputFile); //kapat
    return 0;
}

int isLetter(char ch) 
{
    if( (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z') || ch=='_')
        return 1;
    return 0;
}

int isDigit(char ch) 
{
    if(ch>='0'&&ch<='9')
        return 1;
    return 0;
}

int isOperator(char ch) 
{
    if(ch=='='||ch=='+'||ch=='/'||ch=='-'
            ||ch=='*'||ch=='['||ch==']'
            ||ch=='.'||ch==','||ch=='**'||ch=='('||ch==')')
        return 1;
    return 0;
}

int isComment(char ch)
{
    if(ch=='c' || ch=='C')
        return 1;
    return 0;
}

int isBooleanOp(char *word)

{
    char *booleanOps[9] =
    {
        ".TRUE.", ".AND.",".FALSE.",".OR.",".NOT.",".LT.",".GE.",".EQ.",".NE."
    };
    int i = 0;
    for(i = 0; i < 9; i++)
    {
        if(!strcasecmp(word,booleanOps[i])) 
            return i+1;
    }
    return 0;
}

int isKeyword(char *word)
{
    char *keywords[60] =
    {
        "integer", "array","real","double precision","complex","logical","character",
        "program","write","read","stop","end","int","real","dble","ichar",
        "char","if","endif","then","else","elseif","do","continue","while",
        "enddo","do while","goto","until","subroutine","call","parameter",
        "return","implicit", "entry","common", "close", "assign",
        "backspace", "block data", "equivalence", "external", "format", "function",
        "pause", "print", "rewind",
        "rewrite", "save","sequence","none"
    };

    int i = 0;
    for(i = 0; i < 59; i++)
    {
        if(!strcasecmp(word,keywords[i])) 
            return i+1;
    }
    return 0;
}

int isSpace(char ch)
{
    switch(ch)
    {
    case 9 ... 13:
    case 32:
        return 1;
    }
    return 0;
}

int isString(char ch) 
{
    if(ch=='\'')
        return 1;
    return 0;
}
char fgetcWithCounter(FILE *file)
{
    char c = fgetc(file);
    if(c == '\n')
    {
        line++;
        lastColCount = column;
        column = 0;
    }
    else
        column++;
    return c;
}

void fgoBackwards(FILE *file,int i)
{
    fseek(file,-1*i*sizeof(char),SEEK_CUR);
    if(column == 0)
    {
        line--;
        column = lastColCount;
    }
    else if(column > i)
    {
        column -= i;
    }
    else
    {
        line--;
        column = lastColCount - (i-column);
    }
}

char fgetcWithoutComment(FILE *file)
{
    char p = fgetcWithCounter(file);
    if( p != 'c' && p != 'C')
        return p;

    char c = fgetcWithCounter(file);
    if((p == 'c' && c ==32))
    {
        while(c != '\n')
        {
            c = fgetcWithCounter(file);
        }
        return fgetcWithoutComment(file);
    }
    if((p == 'C' && c ==32))
    {
        while(c != '\n')
        {
            c = fgetcWithCounter(file);
        }
        return fgetcWithoutComment(file);
    }
    fgoBackwards(file,1);
    return p;
}

void printAndWrite(int line, int column, int tokenId, char *tokenValue)
{
    tokenCounts[tokenId]++;
    printf("(%2d,%2d) %15s  %s \n",line,column,token_names[tokenId],tokenValue);
}

void printErrorAndWrite(int line, int col, int errorId, char *value)
{
    printf("ERROR: %s at (%2d,%2d) %s\n",error_strings[errorId],line,col,value);
}

void shortenStringFrom(int i, char *string)
{
    string[i++]='.';
    string[i++]='.';
    string[i++]='.';
    string[i]='\0';
}

void doLexicalAnalysis(FILE *file)
{
    int ln,col,i=0;
    char c;
    char text[MAX_STRING_LENGHT];

    while((c=fgetcWithoutComment(file)) != EOF)
    {
        ln = line;
        col = column;
        if(isLetter(c))
        {
            i = 0;
            while(isLetter(c) || isDigit(c))
            {
                if(i > MAX_VAR_LENGHT)
                {
                    shortenStringFrom(10,text);
                    printErrorAndWrite(ln,col,MAX_VAR_LENGTH_ERROR,text);
                    return;
                }
                text[i++] = c;
                c = fgetcWithoutComment(file);
            }
            text[i] = '\0';

            if(isBooleanOp(text))
                printAndWrite(ln,col,OPERATOR,text);
            else if(isKeyword(text))
                printAndWrite(ln,col,KEYWORD,text);
            else
                printAndWrite(ln,col,IDENTIFIER,text);
            fgoBackwards(file,1);
            continue;
        }

        if(isDigit(c))
        {
            int dot = 0;
            i = 0;
            while(isDigit(c) || (c == '.' && !dot))
            {
                if(c=='.')
                    dot=1;
                text[i++] = c;
                c = fgetcWithoutComment(file);
            }
            text[i] = '\0';
            printAndWrite(ln,col,NUMBER,text);
            fgoBackwards(file,1);
            continue;
        }

        if(isString(c))
        {
            i = 0;
            text[i++] = c;
            while(!isString(c = fgetcWithoutComment(file)))
            {
                if(i > MAX_STRING_LENGHT)
                {
                    shortenStringFrom(10,text);
                    printErrorAndWrite(ln,col,MAX_STRING_LENGTH_ERROR,text);
                    return;
                }
                text[i++] = c;
            }
            text[i++] = c;
            text[i] = '\0';
            printAndWrite(ln,col,STRING,text);
            continue;
        }

        if(isSpace(c))
            continue;

        if(isOperator(c))
        {
            char p = c;
            text[0] = p;
            text[1] = '\0';
            if(p == '*')
            {
                c = fgetcWithoutComment(file);
                if(c != '*')
                {
                    printAndWrite(ln,col,OPERATOR,text);
                    fgoBackwards(file,1);
                    continue;
                }
                text[1] = c;
                text[2] = '\0';
                printAndWrite(ln,col,OPERATOR,text);
                continue;
            }
            else if(p=='.')
            {
                c = fgetcWithoutComment(file);
                if(c != 'L')
                {
                    printAndWrite(ln,col,OPERATOR,text);
                    fgoBackwards(file,1);
                    continue;
                }

                text[1] = c;
                text[2] = '\0';
                printAndWrite(ln,col,OPERATOR,text);
                continue;
            }

            if(p=='(' ||p==')')
            {
                printAndWrite(ln,col,PARANTHESIS,text);
                continue;
            }
            else
            {
                printAndWrite(ln,col,OPERATOR,text);
                continue;
            }
        }
        text[0] = c;
        text[1] = '\0';
        printErrorAndWrite(line,column,UNKNOWN_SYMBOL_ERROR,text);
        return;
    }
}

void printResult()
{
    int i = 0;
    printf("\nNUMBER OF TOKENS \n");
    for(i = 0; i<7 ; i++)
    {
        printf("%-15s : %d\n",token_names[i],tokenCounts[i]);
    }
}


