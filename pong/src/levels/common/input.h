#pragma once

#include "defs.h"
#include "resources.h"
#include "common/res.h"

#include <raylib.h>

#include <numeric>
#include <array>
#include <map>
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

    bool anyData() const {
      return std::reduce(frameKeysDown.begin(), frameKeysDown.end(), false, std::logical_or{})
       || std::reduce(prevFrameKeysDown.begin(), prevFrameKeysDown.end(), false, std::logical_or{}); 
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
    std::map<std::size_t, State> frameData;

    Recorder(Mode m, std::string p): mode(m), path(p) {
      if(mode == Mode::Playback) {
        std::cout << "Reading recording from: " << path << '\n';
        std::ifstream  recording{path};
        recording >> *this;
      }
    }

    Recorder() = delete;
    Recorder(const Recorder& copy) = default;

    Recorder& operator=(Recorder&& move) noexcept {
      mode = move.mode;
      path = std::move(move.path);
      frameData = std::move(move.frameData);

      return *this;
    }

    Recorder(Recorder&& move) noexcept {
      *this = std::move(move);
    }

    ~Recorder() {
      if(mode == Mode::Recording && !path.empty()) {
        std::cout << "Writing recording to: " << path << '\n';
        std::ofstream  recording{path};
        recording << *this;
      }
    }

    friend std::istream& operator>>(std::istream& istream, Recorder& ipr) { 
      std::string line;

      while(std::getline(istream, line)) { 
        std::size_t frame = std::stoi(line);

        State ip;
        istream >> ip;

        // Read the input state at that frame
        ipr.frameData.insert({frame, ip});
      }

      return istream;
    }

    friend std::ostream& operator<<(std::ostream& ostream, const Recorder& ipr) {
      for(const auto& [frame, data]: ipr.frameData)  { ostream << frame << '\n' << data; }
      return ostream;
    };
  };

  static void pollNewInputs(ecs::Resources &global) {
      auto &input = global.getResource<Input>()->get();

      auto ipr_opt   = global.getResource<Recorder>();
      if(ipr_opt && ipr_opt->get().mode == Recorder::Mode::Playback) {
        auto& ipr = ipr_opt->get();
        auto frame = global.getResource<Frame>()->get();

        auto iter = ipr.frameData.find(frame);
        if(iter != ipr.frameData.end()) {
          input.state = iter->second;
        } else {
          input.state = State{};
        }
      } else {
        for (u32 i = 0; i < input.state.frameKeysDown.size(); i++) {
          input.state.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
        }
      }
  }

  static void onFrameEnd(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();
    auto ipr   = global.getResource<Recorder>();
    auto frame   = global.getResource<Frame>()->get();

    if(ipr) {
      switch (ipr->get().mode) {
      case Recorder::Mode::Recording:
        // Record the state before any modifications
        ipr->get().frameData.insert({frame, input.state});
        break;
      case Recorder::Mode::Playback:
        // do nothing and return
        return;
      }
    } 

    input.state.prevFrameKeysDown = input.state.frameKeysDown;
    input.state.frameKeysDown = {};
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
