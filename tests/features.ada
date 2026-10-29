-- Exercises most of SADAL: types, constants, arithmetic, strings, IF chains
PROCEDURE Features IS
    pi : CONSTANT FLOAT := 3.14159;
    r, area : FLOAT;
    a, b : INTEGER := 7;
    name : STRING := "SADAL";
    initial : CHARACTER;
    ok : BOOLEAN := TRUE;
BEGIN
    r := 2.0;
    area := pi * r ** 2;
    Put("area = "); PutLine(area);

    b := (-a) MOD 3;
    Put("-7 mod 3 = "); PutLine(b);
    PutLine(2 ** 10);
    PutLine(2.0 ** -2);
    PutLine(17 / 5);

    initial := name(1);
    PutLine("first letter: " & initial);
    PutLine("slice: " & name(2..4));

    IF a > 10 THEN
        PutLine("big");
    ELSIF a > 5 AND ok THEN
        PutLine("medium");
        IF NOT (a = 7) THEN
            PutLine("never printed");
        ELSE
            PutLine("a is seven");
        END IF;
    ELSE
        PutLine("small");
    END IF;
END Features;
