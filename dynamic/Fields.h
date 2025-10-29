Field(int, A)
Field(float, B)
Field(char, C)
Field(double, Padding1)
Field(float, Padding2)
Field(double, Padding3)
Field(std::string, Name) //<-- !!!!

//FieldPtr(type, name) ?
//see: RAII (instead of using char*, should I use std::string) 