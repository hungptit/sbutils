#include <vector>
#include <iostream>

std::vector<std::vector<int>> GetFloorsWithOneBlock(int width) {
    std::vector<std::vector<int>> floorswithoneblock;
    
    for(int i = 0; i <= width - 2; i++) {
        std::vector<int> floorwithoneblock;
        floorwithoneblock.push_back(i);
        floorwithoneblock.push_back(i+1);
        floorswithoneblock.push_back(floorwithoneblock);
    }
    
    return floorswithoneblock;
}
std::vector<std::vector<int>> AppendBlocksToFloor(std::vector<int> floor, int width) {
    std::vector<std::vector<int>> floorswithadditionalblock;
    
    int lastblock = floor.back();
    for(int i = lastblock+1; i <= width - 2; i++) {
        std::vector<int> floorwithadditionalblock = floor;
        floorwithadditionalblock.push_back(i);
        floorwithadditionalblock.push_back(i+1);
        floorswithadditionalblock.push_back(floorwithadditionalblock);
    }
    
    return floorswithadditionalblock;   
}
std::vector<std::vector<int>> ValidFloors(int width) {
    std::vector<std::vector<int>> floorlist;
    
    std::vector<std::vector<int>> unprocessedfloors = GetFloorsWithOneBlock(width);
    
    while (unprocessedfloors.size() > 0) {
        std::vector<int> floor = unprocessedfloors.back();
        unprocessedfloors.pop_back();
        
        floorlist.push_back(floor);
        std::vector<std::vector<int>> additionalfloors = AppendBlocksToFloor(floor, width);
        unprocessedfloors.insert( unprocessedfloors.end(), additionalfloors.begin(), additionalfloors.end() );
    }
    
    return floorlist;  
}
int main() {
    std::cout << ValidFloors(4);
}
