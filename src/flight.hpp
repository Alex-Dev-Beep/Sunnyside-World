#include <vector>
#include <vulkan/vulkan.h>

struct flightFrames {
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    const int MAX_FRAMES_IN_FLIGHT = 2; 
};

extern flightFrames FlightFrames;