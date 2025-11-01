# Variables
CXX = g++
CXXFLAGS = -Iinclude
LIBS = -lglfw -lGL -lm -ldl
SRCDIR = src
BUILDDIR = build
INCLUDEDIR = include

# Object files
OBJS = $(BUILDDIR)/main.o $(BUILDDIR)/glad.o

# Main target
$(BUILDDIR)/main: $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

# Object file rules
$(BUILDDIR)/main.o: $(SRCDIR)/main.cc $(INCLUDEDIR)/game.h $(INCLUDEDIR)/shader.h $(INCLUDEDIR)/utils.h $(INCLUDEDIR)/glad/glad.h
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/glad.o: $(SRCDIR)/glad.c $(INCLUDEDIR)/glad/glad.h $(INCLUDEDIR)/KHR/khrplatform.h
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Convenience targets
build: $(BUILDDIR)/main

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/main

.PHONY: build clean