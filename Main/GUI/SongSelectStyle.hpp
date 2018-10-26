#pragma once
#include "Shared/Jobs.hpp"



class SongSelectStyle : public Unique
{
public:
	SongSelectStyle(class Application* application);
	~SongSelectStyle();
	static Ref<SongSelectStyle> Get(class Application* application = nullptr);
	static Ref<SongSelectStyle> instance;

	Texture frameMain;
	Texture frameSub;
	// 5 Difficulty frame textures for the following difficulties
	// 1 = Novice
	// 2 = Advanced
	// 3 = Exhaust
	// 4 = Gravity
	// 5 = Infinite
	static const size_t numDiffFrames = 5;
	Texture diffFrames[numDiffFrames];

	// Loading image
	Texture loadingJacketImage;

	// Material used for compositing difficulty frames with jacket images
	Material diffFrameMaterial;

	// Cached jacket images
	Texture GetJacketThumnail(const String& path);

private:
	Timer m_timer;
	Map<String, class CachedJacketImage*> m_jacketImages;
};