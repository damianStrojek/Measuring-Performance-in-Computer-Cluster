# How-to check for running process

While developing we decided to delete checking whether specific process ID is still running or not.

You may want to implement it. All you need to do is:

## 1. Create function to check this specific process ID

```cpp
const char* dateCommand = "date +'%d%m%y-%H%M%S'",
        *processCommand = "ps -p 1 > /dev/null && echo '1' || echo '0'";	// [TODO] Add GPROCESSID
```

## 2. Revert changes to while function

You need to replace our for function with `while()` and you may add exiting the program on keyboard hit. 

```cpp
// Checking if process with GPROCESSID is still running
while(std::stoi(exec(processCommand))){
...
}
```

```cpp
if(keyboardHit()){
	std::cout << "\n\n\t[STOP] Key pressed.\n\n";
        break;
}
```

```cpp
int keyboardHit(void);

// Actively checking for the user input to break from the main while loop
int keyboardHit(void){
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
};
```