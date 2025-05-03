TARGET = rat25s
SRC = main.cpp
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

make:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o Syntax_Output.txt Lexical_Analysis_Output.txt RPD_File.txt
