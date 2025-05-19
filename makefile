# Variabili C
C_SOURCES := CSources
C_HEADERS := CHeaders
C_OBJECTS := CObjects
C_SOURCE_FILES := $(wildcard $(C_SOURCES)/*.c)
C_OBJECT_FILES := $(patsubst $(C_SOURCES)/%.c,$(C_OBJECTS)/%.o,$(C_SOURCE_FILES))
C_EXECUTABLE := cammini.out
C_FLAGS := -O3 -march=native -pthread -I$(C_HEADERS)

# Variabili Java
J_SOURCES := javaSources
J_SOURCE_FILES := $(wildcard $(J_SOURCES)/*.java) 
J_MAINCLASS := CreaGrafo

# Target default
all: $(C_EXECUTABLE) java_compile

# Compilazione C
$(C_OBJECTS)/%.o: $(C_SOURCES)/%.c | $(C_OBJECTS)
	gcc $(C_FLAGS) -c $< -o $@

$(C_EXECUTABLE): $(C_OBJECT_FILES)
	gcc $(C_FLAGS) $^ -o $@

$(C_OBJECTS):
	mkdir -p $(C_OBJECTS)

# Compilazione Java
java_compile: $(J_SOURCE_FILES)
	javac -d . $(J_SOURCE_FILES)

# Clean-up
clean:
	rm -rf $(C_OBJECTS) $(C_EXECUTABLE)
	rm -f *.class

.PHONY: all java_compile clean
