#pragma once

#include "defs.h"
#include "resources.h"
#include "common/res.h"

#include <raylib.h>

#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

namespace pong {
struct Input {
  struct State { 
    std::array<bool, 348> frameKeysDown, prevFrameKeysDown; 

    State() { 
        for(auto& k: frameKeysDown) k = false;
        for(auto& k: prevFrameKeysDown) k = false;
    }

    // First comment in https://stackoverflow.com/a/14266139
    static std::vector<std::string> split(const std::string& line, char delim) {
      if(line.empty()) return {};

      size_t last = 0; 
      size_t next = 0; 
      std::vector<std::string> parts;

      while ((next = line.find(delim, last)) != std::string::npos) {   
        parts.push_back(line.substr(last, next-last));
        last = next + 1; 
      } 
      
      // Avoid adding empty strings
      if(last < line.length() - 1) {
        parts.push_back(line.substr(last));
      }

      return parts;
    }

    static void writeLine(std::ostream& ostream, const std::array<bool, 348>& array) {
      for(u32 i = 0; i < array.size(); i++) {
        if(array[i] == true) { ostream << i << ','; }
      }
      ostream << '\n';
    }

    static void readLine(std::istream& istream, std::array<bool, 348>& array) {
      std::string line;
      std::getline(istream, line);
      for(const auto& keyString: split(line, ',')) {
        u32 input = std::stoi(keyString);
        array[input] = true;
      }
    }

    friend std::ostream& operator<<(std::ostream& ostream, const State& ip) {
      writeLine(ostream, ip.frameKeysDown);
      writeLine(ostream, ip.prevFrameKeysDown);
      return ostream;
    }

    friend std::istream& operator>>(std::istream& istream, State& ip) {
      readLine(istream, ip.frameKeysDown);
      readLine(istream, ip.prevFrameKeysDown);

      return istream;
    }
  };

  struct Recorder {
    enum struct Mode { Recording, Playback };

    Mode mode;
    std::string path;
    std::vector<State> frameData;

    Recorder(Mode m, std::string p): mode(m), path(p) {
      if(mode == Mode::Playback) {
        std::cout << "Reading recording from: " << path << '\n';
        std::ifstream  recording{path};
        recording >> *this;
      }
    }

    ~Recorder() {
      if(mode == Mode::Recording) {
        std::cout << "Writing recording to: " << path << '\n';
        std::ofstream  recording{path};
        recording << *this;
      }
    }

    friend std::istream& operator>>(std::istream& istream, Recorder& ipr) { 
      std::string line;
      State ip;

      while(istream >> ip) { 
        // Read the input state at that frame
        ipr.frameData.push_back(ip);

        std::getline(istream, line); // Skip the newline between entires
      }

      return istream;
    }

    friend std::ostream& operator<<(std::ostream& ostream, const Recorder& ipr) {
      for(const auto& ip: ipr.frameData)  { ostream << ip << '\n'; }
      return ostream;
    };
  };

  static void pollNewInputs(ecs::Resources &global) {
      auto &input = global.getResource<Input>()->get();

      auto ipr   = global.getResource<Recorder>();
      if(ipr && ipr->get().mode == Recorder::Mode::Playback) {
        auto frame = global.getResource<Frame>()->get();
        input.state = ipr->get().frameData.at(frame);
      } else {
        for (u32 i = 0; i < input.state.frameKeysDown.size(); i++) {
          input.state.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
        }
      }
  }

  static void onFrameEnd(ecs::Resources &global) {
    auto &input = global.getResource<Input>().value().get();
    auto ipr   = global.getResource<Recorder>();

    if(ipr) {
      switch (ipr->get().mode) {
      case Recorder::Mode::Recording:
        // Record the state before any modifications
        ipr->get().frameData.push_back(input.state); 
        break;
      case Recorder::Mode::Playback:
        // do nothing and return
        return;
      }
    } 

    input.state.prevFrameKeysDown = input.state.frameKeysDown;
    for (auto &f : input.state.frameKeysDown) { f = false; }
  }

  bool isKeyDown(KeyboardKey k) const { return state.frameKeysDown[static_cast<u32>(k)]; }

  bool wasKeyPressed(KeyboardKey k) const {
    return state.frameKeysDown[static_cast<u32>(k)] && !state.prevFrameKeysDown[static_cast<u32>(k)];
  }
private:
  // Raylib's `KeyboardKey` enum has a maximum value of 348.
  State state;
};
}
