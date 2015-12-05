# SHAUN

C++ implementation of SHAUN (Shaun HAtes Ugly Notations)
an object notation highly inspired by HJSON (http://hjson.org)

# Usage

SHAUN can store data representing strings, numbers, or booleans.

```
/*
  here is a simple SHAUN file
  comments can be written with parenthesis
  or like C notation
*/

object: {
  pi: 3.14 ( every number is a floating point number )
  
  angle: 0 rad // you can specify units for numbers
  text: "hello there"

  ( lists can contain differents types of elements )
  list: [ "i'm " 20 " years old" ]
  IsItTrue: true
  
  another_object: { hello: "there", iam: "John" }
}
```

To access elements, you can either use the shaun objects methods
or use the sweeper object.

```C++
int main(void)
{
  using namespace shaun;
  
  // parsing a .sn file
  parser p;
  object o = p.parse_file("file.sn");
  
  // searching for a value
  sweeper sw(&o);
  
  std::cout << "I am " << sw.get("object:list[1]").value<number>() << " years old\n";
  
  // copying a sweeper
  sweeper another_sweeper(sw.get("object:another_object"));
  
  std::cout << "My name is " << another_sweeper.get("iam").value<string>() << " !\n";
}
```