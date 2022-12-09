#pragma once


#include <efsw/efsw.hpp>

namespace lcdoc
{
	// Inherits from the abstract listener class, and implements the the file action handler
	class FuncitonalUpdateListener final : public efsw::FileWatchListener {
	public:

		using Listener = std::function<void(efsw::WatchID watchid, const std::string& dir,
			const std::string& filename, efsw::Action action,
			std::string oldFilename)>;

		Listener listener;
		std::function<void(void)> boldLstener;

		void handleFileAction(efsw::WatchID watchid, const std::string& dir,
			const std::string& filename, efsw::Action action,
			std::string oldFilename) override {

			if (this->listener)
				this->listener(watchid, dir, filename, action, oldFilename);

			if (this->boldLstener)
				this->boldLstener();
		}
	};
}