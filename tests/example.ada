PROCEDURE Example IS
    x, y : INTEGER;
    result : FLOAT := 3.14;
BEGIN
    Get(x);
    IF x > 10 THEN
        y := x * 2 + 5;
        PutLine(y);
    ELSIF x < 0 THEN
        y := -x;
        Put(y);
    ELSE
        y := x;
    END IF;
END Example;
