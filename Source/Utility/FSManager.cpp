#include <JuceHeader.h>
#include "FSManager.h"

File FSManager::documents = File(File::getSpecialLocation(File::userDocumentsDirectory));
File FSManager::home = documents.getChildFile("Cerite");
File FSManager::media = home.getChildFile("Media");
File FSManager::work = home.getChildFile(".work"); // hidden working directory
File FSManager::metaData = home.getChildFile("metadata.xml");

std::unique_ptr<XmlElement> FSManager::meta = parseXML(metaData.loadFileAsString());


void FSManager::createFilesystem()
{
    if(!work.exists()) {
        work.createDirectory();
    }
	if(!home.exists())
	{
		MemoryInputStream inputstream = MemoryInputStream(BinaryData::Cerite_zip, BinaryData::Cerite_zipSize, false);
		ZipFile unzipper = ZipFile(inputstream);
		unzipper.uncompressTo(File(File::getSpecialLocation(File::userDocumentsDirectory)));
	}

	if(!metaData.exists())
	{
		std::unique_ptr<XmlElement> XmlData = std::make_unique<XmlElement>("Meta");
		XmlElement* lastOpened = new XmlElement("lastProject");
		XmlElement* recentlyOpened = new XmlElement("recentlyOpened");
		XmlData->addChildElement(recentlyOpened);
		XmlData->addChildElement(lastOpened);
		String fileString = XmlData->toString();
		metaData.create();
		metaData.replaceWithText(fileString);
	}
}
