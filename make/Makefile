
CXX=g++
CXXFLAGS=-Wall -Wextra -O2
DIR_BUILD=../build
DIR_SOURCE=..

SRCS_CLIENT_TCP=$(DIR_SOURCE)/client_tcp.cpp \
                 $(DIR_SOURCE)/protocol.cpp

OBJS_CLIENT_TCP=$(SRCS_CLIENT_TCP:$(DIR_SOURCE)/%.cpp=$(DIR_BUILD)/%.o)

SRCS_CLIENT_UDP=$(DIR_SOURCE)/client_udp.cpp \
                $(DIR_SOURCE)/protocol.cpp

OBJS_CLIENT_UDP=$(SRCS_CLIENT_UDP:$(DIR_SOURCE)/%.cpp=$(DIR_BUILD)/%.o)

SRCS_SERVER=$(DIR_SOURCE)/server.cpp   \
            $(DIR_SOURCE)/protocol.cpp \
            $(DIR_SOURCE)/epoll.cpp

OBJS_SERVER=$(SRCS_SERVER:$(DIR_SOURCE)/%.cpp=$(DIR_BUILD)/%.o)

all: $(DIR_BUILD)/kart-client_tcp \
     $(DIR_BUILD)/kart-client_udp \
     $(DIR_BUILD)/kart-server

$(DIR_BUILD)/kart-client_tcp: $(OBJS_CLIENT_TCP)
	$(CXX) $(OBJS_CLIENT_TCP) $(LDFLAGS) -o $(DIR_BUILD)/kart-client_tcp

$(DIR_BUILD)/kart-client_udp: $(OBJS_CLIENT_UDP)
	$(CXX) $(OBJS_CLIENT_UDP) $(LDFLAGS) -o $(DIR_BUILD)/kart-client_udp

$(DIR_BUILD)/kart-server: $(OBJS_SERVER)
	$(CXX) $(OBJS_SERVER) $(LDFLAGS) -o $(DIR_BUILD)/kart-server

$(DIR_BUILD)/%.o: $(DIR_SOURCE)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
