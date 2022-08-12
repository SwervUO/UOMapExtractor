//Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef mapblock_hpp
#define mapblock_hpp

#include <cstdint>
#include <string>
#include <utility>
#include <tuple>

#include <vector>

//=================================================================================
//		Terrain type structures
//=================================================================================
//=================================================================================
class terrainblock_t {
	
	std::vector<std::uint8_t> blockdata ;
	
public:
	terrainblock_t(std::int32_t blockheader) ;
	terrainblock_t(const std::uint8_t* data = nullptr);
	
	auto header() const ->std::int32_t ;
	auto header(std::int32_t value) ->void;

	auto raw() const -> const std::vector<std::uint8_t>& ;
	auto raw()  -> std::vector<std::uint8_t>& ;

	auto terrain(int x, int y) const -> std::pair<std::uint16_t,std::int8_t> ;
	auto terrain(int x, int y, std::uint16_t tileid, std::int8_t altitude) ->void ;
	auto fill(std::uint16_t tileid, std::int8_t altitude) ->void ;
};

//=================================================================================
//		Art type structures
//=================================================================================


//=================================================================================
class artblock_t {
	std::vector<std::uint8_t> blockdata ;
	
public:
	artblock_t(const std::uint8_t *data, size_t size) ;
	artblock_t() ;
	
	auto size() const ->size_t ;
	
	auto raw() const -> const std::vector<std::uint8_t>& ;
	auto raw()  -> std::vector<std::uint8_t>& ;
	auto clear() ->void ;
	
	auto art(int x, int y, std::uint16_t tileid, std::int8_t altitude, std::uint16_t hue =0 ) ->void ;
	auto art(int x, int y) const -> std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> ;
	auto art(int x, int y,int alt) const -> std::vector<std::tuple<std::uint16_t,std::int8_t,std::uint16_t>> ;
	auto remove(int x, int y) ->void ;
	auto remove(int x, int y,int alt) ->void ;
};

#endif /* mapblock_hpp */
