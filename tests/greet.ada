PROCEDURE Greet IS
    who : STRING;
    age : INTEGER;
BEGIN
    Put("Name? ");
    Get(who);
    Put("Age? ");
    Get(age);
    PutLine("Hello, " & who & "!");
    IF age >= 18 THEN
        PutLine("You are an adult.");
    ELSE
        PutLine("You are a minor.");
    END IF;
END Greet;
