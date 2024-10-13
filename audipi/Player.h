#ifndef PLAYER_H
#define PLAYER_H
#include <string>

#include "structs.h"

namespace audipi {
    class Player {
        struct player_track {
            std::string name;
            msf_location location;
            msf_location duration;
        };

    public:
    };
}

#endif //PLAYER_H
