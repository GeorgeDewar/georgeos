// Uh-oh. If we put this inside the method, it gets stored on the stack,
// and bp is used to refer to it which overwrites our function code or something
// Edit: Fixed with this commit, but more lines :(

void appMain(void)
{
    char name[16];
    println("Hello, what is your name?");
    print("Name: ");
    readString(name, 1);
    print("Nice to meet you, ");
    println(name);
    println("Goodbye");
}
