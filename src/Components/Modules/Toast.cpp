#include "STDInc.hpp"

namespace Components
{
	std::queue<Toast::UIToast> Toast::Queue;
	std::mutex Toast::Mutex;

	void Toast::Show(const std::string& image, const std::string& title, const std::string& description, int length, bool special, const Utils::Slot<void()>& callback)
	{
		Game::Material* material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, image.data()).material;
		return Show(material, title, description, length, special, callback);
	}

	void Toast::Show(Game::Material* material, const std::string& title, const std::string& description, int length, bool special,const Utils::Slot<void()>& callback)
	{
		std::lock_guard _(Mutex);
		Queue.push({ material, (Language::GetCurrentLanguage() != "french") ? Utils::String::ToUpper(title) : title, description, length, 0, special, callback });
		//Queue.push({ material, Utils::String::ToUpper(title), description, length, 0, special, callback });
	}

	void Toast::Draw(UIToast* toast)
	{
		if (!toast) return;

		int width = Renderer::Width();
		int height = Renderer::Height();

		// Define reference resolution
		const int refWidth = 640;
		const int refHeight = 480;

		// Calculate scaling factor
		float scaleX = static_cast<float>(width) / refWidth;
		float scaleY = static_cast<float>(height) / refHeight;

		// Use the smaller scaling factor to maintain aspect ratio
		float scale = std::min(scaleX, scaleY);

		// Apply additional scaling to make elements smaller
		float sizeReductionFactor = 0.8f;
		scale *= sizeReductionFactor;

		if (toast->special)
		{
			int slideTime = 300;

			int duration = toast->length;
			int startTime = toast->start;

			int border = static_cast<int>(1 * scale);
			int cornerSize = static_cast<int>(15 * scale);
			int bHeight = static_cast<int>(74 * scale);

			int imgDim = static_cast<int>(60 * scale);
			float fontSize = 0.0f;
			float descSize = 0.0f;

			if (Language::GetCurrentLanguage() == "russian")
			{
				fontSize = 1.05f * scale;
				descSize = 1.05f * scale;
			}
			else
			{
				fontSize = 0.55f * scale;
				descSize = 0.75f * scale;
			}

			int xPos = 0;

			Game::vec4_t wColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			Game::vec4_t bgColor = { 0.0f, 0.0f, 0.0f, 0.8f };
			Game::vec4_t borderColor = { 1.0f, 1.0f, 1.0f, 0.2f };

			height /= 5;
			height *= 0.5;

			if (Game::Sys_MilliSeconds() < startTime || (startTime + duration) < Game::Sys_MilliSeconds()) return;

			// Fadein stuff
			if (Game::Sys_MilliSeconds() - startTime < slideTime)
			{
				int diffW = -static_cast<int>(1000 * scale);
				int diff = Game::Sys_MilliSeconds() - startTime;
				double fadeScale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
				diffW = static_cast<int>(fadeScale * -1000 * scale);
				width += diffW;
				xPos += diffW;
			}

			// Fadeout stuff
			else if (Game::Sys_MilliSeconds() - startTime > (duration - slideTime))
			{
				int diffW = 0;
				int diff = (startTime + duration) - Game::Sys_MilliSeconds();
				double fadeScale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
				diffW = static_cast<int>(fadeScale * -1000 * scale);
				width += diffW;
				xPos += diffW;
			}

			height += bHeight / 2 - cornerSize;

			// Calculate width data
			int iOffset = (bHeight - imgDim) / 2;
			int iOffsetLeft = iOffset * 2;
			float titleSize = Game::R_TextWidth(toast->title.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.objectiveFont) * fontSize;
			float descrSize = Game::R_TextWidth(toast->desc.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.bigFont) * descSize;

			float bWidth = iOffsetLeft * 3 + imgDim + std::max(titleSize, descrSize);

			bWidth = (static_cast<int>(bWidth) + (static_cast<int>(bWidth) % 2)) * 1.0f;
			bHeight += (bHeight % 2);

			// Background
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(xPos), static_cast<float>(height - bHeight / 2), 400.0f * scale, bHeight * 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, bgColor);

			// Border
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(xPos), static_cast<float>(height - bHeight / 2 - border), border * 1.0f, bHeight + (border * 2.0f), 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Left
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(xPos + 400.f * scale), static_cast<float>(height - bHeight / 2 - border), border * 1.0f, bHeight + (border * 2.0f), 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Right
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(xPos), static_cast<float>(height - bHeight / 2 - border), 400.0f * scale, border * 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Top
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(xPos), static_cast<float>(height + bHeight / 2), 400.0f * scale, border * 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Bottom

			// Image
			Game::Material* image = toast->image;
			Game::R_AddCmdDrawStretchPic(image, static_cast<float>(xPos + iOffsetLeft), static_cast<float>(height - bHeight / 2 + iOffset), imgDim * 1.0f, imgDim * 1.0f, 0, 0, 1.0f, 1.0f, wColor);

			// Text
			float title = xPos + 80.0f * scale;
			float desc = xPos + 80.0f * scale;

			Game::R_AddCmdDrawTextASM(toast->title.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.objectiveFont, static_cast<float>(title), static_cast<float>(height - bHeight / 2 + cornerSize * 2 + 7 * scale), fontSize, fontSize, 0, wColor, 6); // Title
			Game::Font_s* fontHandle = Dvars::console_menu_style->current.enabled ? Game::sharedUiInfo->assets.smallFont : Game::sharedUiInfo->assets.bigFont;
			Game::R_AddCmdDrawTextASM(toast->desc.data(), std::numeric_limits<int>::max(), fontHandle, static_cast<float>(desc), static_cast<float>(height - bHeight / 2 + cornerSize * 2 + 33 * scale), descSize, descSize, 0, wColor, 6); // Description
		}
		else
		{
			int slideTime = 100;

			int duration = toast->length;
			int startTime = toast->start;

			int border = 1;
			int cornerSize = 15;
			int bHeight = 74;

			int imgDim = 60;

			float fontSize = 0.9f;
			float descSize = 0.9f;

			Game::vec4_t wColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			Game::vec4_t bgColor = { 0.0f, 0.0f, 0.0f, 0.8f };
			Game::vec4_t borderColor = { 1.0f, 1.0f, 1.0f, 0.2f };

			height /= 5;
			height *= 4;

			if (Game::Sys_MilliSeconds() < startTime || (startTime + duration) < Game::Sys_MilliSeconds()) return;

			// Fadein stuff
			if (Game::Sys_MilliSeconds() - startTime < slideTime)
			{
				int diffH = Renderer::Height() / 5;
				int diff = Game::Sys_MilliSeconds() - startTime;
				scale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
				diffH = static_cast<int>(diffH * scale);
				height += diffH;
			}

			// Fadeout stuff
			else if (Game::Sys_MilliSeconds() - startTime > (duration - slideTime))
			{
				int diffH = Renderer::Height() / 5;
				int diff = (startTime + duration) - Game::Sys_MilliSeconds();
				scale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
				diffH = static_cast<int>(diffH * scale);
				height += diffH;
			}

			height += bHeight / 2 - cornerSize;

			// Calculate width data
			int iOffset = (bHeight - imgDim) / 2;
			int iOffsetLeft = iOffset * 2;
			float titleSize = Game::R_TextWidth(toast->title.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.objectiveFont) * fontSize;
			float descrSize = Game::R_TextWidth(toast->desc.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.bigFont) * descSize;
			float bWidth = iOffsetLeft * 3 + imgDim + std::max(titleSize, descrSize);

			// Make stuff divisible by 2
			// Otherwise there are overlapping images
			// and I'm too lazy to figure out the actual problem :P
			bWidth = (static_cast<int>(bWidth) + (static_cast<int>(bWidth) % 2)) * 1.0f;
			bHeight += (bHeight % 2);

			// Background
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(width / 2 - bWidth / 2), static_cast<float>(height - bHeight / 2), bWidth * 1.0f, bHeight * 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, bgColor);

			// Border
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(width / 2 - bWidth / 2 - border), static_cast<float>(height - bHeight / 2 - border), border * 1.0f, bHeight + (border * 2.0f), 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Left
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(width / 2 - bWidth / 2 + bWidth), static_cast<float>(height - bHeight / 2 - border), border * 1.0f, bHeight + (border * 2.0f), 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Right
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(width / 2 - bWidth / 2), static_cast<float>(height - bHeight / 2 - border), bWidth * 1.0f, border * 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Top
			Game::R_AddCmdDrawStretchPic(Game::cgMedia->whiteMaterial, static_cast<float>(width / 2 - bWidth / 2), static_cast<float>(height + bHeight / 2), bWidth * 1.0f, border * 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, borderColor); // Bottom


			// Image
			Game::Material* image = toast->image;

			Game::R_AddCmdDrawStretchPic(image, static_cast<float>(width / 2 - bWidth / 2 + iOffsetLeft), static_cast<float>(height - bHeight / 2 + iOffset), imgDim * 1.0f, imgDim * 1.0f, 0, 0, 1.0f, 1.0f, wColor);

			// Text
			float leftText = width / 2 - bWidth / 2 - cornerSize + iOffsetLeft * 2 + imgDim;
			float rightText = width / 2 + bWidth / 2 - cornerSize - iOffsetLeft;
			Game::R_AddCmdDrawTextASM(toast->title.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.objectiveFont, static_cast<float>(leftText + (rightText - leftText) / 2 - titleSize / 2 + cornerSize), static_cast<float>(height - bHeight / 2 + cornerSize * 2 + 7), fontSize, fontSize, 0, wColor, 6); // Title
			Game::R_AddCmdDrawTextASM(toast->desc.data(), std::numeric_limits<int>::max(), Game::sharedUiInfo->assets.bigFont, leftText + (rightText - leftText) / 2 - descrSize / 2 + cornerSize, static_cast<float>(height - bHeight / 2 + cornerSize * 2 + 33), descSize, descSize, 0, wColor, 6); // Description
		}
	}

	void Toast::Handler()
	{
		if (Queue.empty())
		{
			return;
		}

		std::lock_guard _(Mutex);

		UIToast* toast = &Queue.front();

		toast->handledOnce = false;
		// Set start time
		if (!toast->start)
		{
			toast->start = Game::Sys_MilliSeconds();
			if (!toast->handledOnce)
			{
				toast->handledOnce = true;
				Command::Execute("snd_playlocal achievement_unlocked_xbox");
			}
		}

		if ((toast->start + toast->length) < Game::Sys_MilliSeconds())
		{
			if (toast->callback) toast->callback();
			Queue.pop();
		}
		else
		{
			Draw(toast);
		}
	}

	Toast::Toast()
	{
		Scheduler::Loop(Handler, Scheduler::Pipeline::RENDERER);
	}

	Toast::~Toast()
	{
	}
}