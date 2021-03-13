#include "pch.h"
#include "BMLoadout.h"

void BMLoadout::write_loadout(BitBinaryWriter<unsigned char>& writer, std::map<uint8_t, Item> loadout)
{
	//Save current position so we can write the length here later
	const int amountStorePos = writer.current_bit;
	//Reserve 4 bits to write size later
	writer.WriteBits(0, 4);

	//Counter that keeps track of size
	int loadoutSize = 0;
	for (auto opt : loadout)
	{
		//In bakkesmod, when unequipping the productID gets set to 0 but doesn't
		//get removed, so we do this check here.
		if (opt.second.product_id <= 0)
			continue;
		loadoutSize++;
		writer.WriteBits(opt.first, 5); //Slot index, 5 bits so we get slot upto 31
		writer.WriteBits(opt.second.product_id, 13); //Item id, 13 bits so upto 8191 should be enough
		writer.WriteBool(opt.second.paint_index > 0); //Bool indicating whether item is paintable or not
		if (opt.second.paint_index > 0) //If paintable
		{
			writer.WriteBits(opt.second.paint_index, 6); //6 bits, allow upto 63 paints
		}
	}

	//Save current position of writer
	const int amountStorePos2 = writer.current_bit;
	writer.current_bit = amountStorePos;
	//Write the size of the loadout to the spot we allocated earlier
	writer.WriteBits(loadoutSize, 4); //Gives us a max of 15 customizable slots per team
	writer.current_bit = amountStorePos2; //Set back reader to original position
}

void BMLoadout::write_color(BitBinaryWriter<unsigned char>& writer, RGB color)
{
	writer.WriteBits(color.r, 8);
	writer.WriteBits(color.g, 8);
	writer.WriteBits(color.b, 8);
}

std::string BMLoadout::save(BMLoadout loadout)
{
	//Allocate buffer thats big enough
	BitBinaryWriter<unsigned char> writer(10000);
	writer.WriteBits(CURRENT_LOADOUT_VERSION, 6); //Write current version

	/*
	We write 18 empty bits here, because we determine size and CRC after writing the whole loadout
	but we still need to allocate this space in advance
	*/
	writer.WriteBits(0, 18);

	writer.WriteBool(loadout.body.blue_is_orange); //Write blue == orange?
	write_loadout(writer, loadout.body.blue_loadout);

	writer.WriteBool(loadout.body.blueColor.should_override); //Write override blue car colors or not

	if (loadout.body.blueColor.should_override)
	{
		write_color(writer, loadout.body.blueColor.primary_colors); // write primary colors RGB (R = 0-255, G = 0-255, B = 0-255)
		write_color(writer, loadout.body.blueColor.secondary_colors); //write secondary
	}

	if (!loadout.body.blue_is_orange)
	{
		write_loadout(writer, loadout.body.orange_loadout);
		writer.WriteBool(loadout.body.orangeColor.should_override);//Write override orange car colors or not
		if (loadout.body.orangeColor.should_override)
		{
			write_color(writer, loadout.body.orangeColor.primary_colors); //write primary
			write_color(writer, loadout.body.orangeColor.secondary_colors); //write secondary
		}
	}

	const int currentBit = writer.current_bit; //Save current location of writer

	int sizeInBytes = currentBit / 8 + (currentBit % 8 == 0 ? 0 : 1); //Calculate how many bytes are used
	writer.current_bit = 6; //Set writer to header (bit 6)
	writer.WriteBits(sizeInBytes, 10); //Write size
	writer.WriteBits(writer.CalculateCRC(3, sizeInBytes), 8); //Write calculated CRC
	writer.current_bit = currentBit; //Set writer back to original position
	return writer.ToHex();
}