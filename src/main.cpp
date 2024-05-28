#include "JsonParser.h"



/*
Key: A key is always a string enclosed in quotation marks.
Value: A value can be a string, number, boolean expression, array, object, or null.

Every key-value pair is separated by a comma.

example 
{ } //Empty JSON object

{
	 “StringProperty”: “StringValue”,
 	“NumberProperty”: 10,
 	“FloatProperty”: 20.13,
 	“BooleanProperty”: true,
 	“EmptyProperty”: null
}

{
	“NestedObjectProperty”: {
		“Name”: “Neste  Object”
	},
	“NestedArrayProperty”: [10,20,true,40]
}
*/
int main() {
    std::string jsonString = R"({"name": "John", "age": 30, "isStudent": false, "courses": ["Math", "Science"]})";
    JsonParser parser(jsonString);
    JsonValue jsonValue = parser.parse();

    printJsonValue(jsonValue);
    return 0;
}
