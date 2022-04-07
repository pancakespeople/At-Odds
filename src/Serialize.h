#pragma once

#define SERIALIZE friend class boost::serialization::access; template<class Archive> void serialize(Archive& archive, const unsigned int version)