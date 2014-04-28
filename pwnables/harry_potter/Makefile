CXXFLAGS = -Os -g -fstack-protector
LDFLAGS =
OBJECTS = except.o aes256.o SHA256.o

.PHONY: clean

except: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	$(RM) $(OBJECTS)

