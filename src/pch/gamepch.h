#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include <random>
#include <thread>
#include <chrono>
#include <string>
#include <algorithm>
#include <memory>
#include <functional>

#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <set>
#include <stack>
#include <queue>

#include <SFML/Main.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <TGUI/TGUI.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/map.hpp>

#include "../Debug.h"
#include "../Serialize.h"

#include "../ext/toml.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include "../ext/sol.hpp"