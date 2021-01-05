#pragma once
#include <JuceHeader.h>


struct FSManager
{

	static File documents;

	static File home;
    static File work;
	static File media;
    static File exec;
	static File metaData;

	static std::unique_ptr<XmlElement> meta;

	static void createFilesystem();

};

