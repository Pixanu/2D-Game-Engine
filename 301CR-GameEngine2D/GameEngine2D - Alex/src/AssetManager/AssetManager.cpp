#include "./AssetManager.h"
#include "../Logger/Logger.h"
#include <SDL_image.h>

AssetManager::AssetManager()
{
	Logger::Log("AssetManager constructor called!");
}

AssetManager::~AssetManager()
{
	ClearAssets();
	Logger::Log("AssetManager destructor called!");
}

void AssetManager::ClearAssets()
{
	for (auto texture : textures) {
		SDL_DestroyTexture(texture.second);
	}
	textures.clear();

	for (auto font : fonts) {
		TTF_CloseFont(font.second);
	}
	fonts.clear();
}

void AssetManager::AddTextures(SDL_Renderer* renderer , const std::string& assetId, const std::string& filepath)
{
	SDL_Surface* surface = IMG_Load(filepath.c_str());
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	//Add the texture to the map
	textures.emplace(assetId, texture);

	Logger::Log("New texture added to the Asset Manager with id = " + assetId);
}

SDL_Texture* AssetManager::GetTexture(const std::string& assetId)
{
	return textures[assetId];
}

void AssetManager::AddFont(const std::string& assetId, const std::string& filePath, int fontSize) {
	fonts.emplace(assetId, TTF_OpenFont(filePath.c_str(), fontSize));
}

TTF_Font* AssetManager::GetFont(const std::string& assetId) {
	return fonts[assetId];
}