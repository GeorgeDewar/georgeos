char name[16]; // Uh-oh. If we put this inside the method, it gets stored on the stack,
// and bp is used to refer to it which overwrites our function code or something

void appMain(void)
{
    println("Hello, what is your name?");
    print("Name: ");
    readString(name, 1);
    print("Nice to meet you, ");
    println(name);
    println("Goodbye");
}
