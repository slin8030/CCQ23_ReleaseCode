/*//add comment for SSID/SVID restore

1. Need HDA device disable(0x488[25]=0) after dGPU power up at _PR3 _ON
2. No need dGPU resource save/restore and only requires dGPU SSID/SSVID save/restore at _PR3 _OFF/_ON
   SSID/SSVID save/restore method: (It can be done by using SMI with legacy IO access) 
   a. Used MMIO access 
   b. Write root port CMD register (offset 0x4) [1:0] = b11 (11 = IO/Memory enable) 
   c. Write root port -PM State [1:0] = b00 (00 = D0) 
   d. SSID/SSVID offset is at dGPU PCIE 0ffset 0x40. 
3. No _STA define at root port.
*/

    Scope(\_SB)
	{
		Name(OSCI, Zero)
		Name(OSCO, Zero)
		Method(_OSC, 4, Serialized)
		{
			CreateDWordField(Arg3, Zero, STS0)
			CreateDWordField(Arg3, 0x04, CAP0)
			If(LEqual(Arg0, ToUUID("0811B06E-4A27-44F9-8D60-3CBBC22E7B48")))
			{
				If(LEqual(Arg1, One))
				{
					If(And(CAP0, 0x20))
					{
							And(CAP0, 0x1F, CAP0)
							Or(STS0, 0x10, STS0)
					}
                    If (\_OSI ("Windows 2013")) {
          					  Or(CAP0, 0x04, CAP0)   // Enable _PR3 support 
          					  Store(STS0, OSCI)
          					  Store(CAP0, OSCO)
                    }
				}
				Else
				{
					And(STS0, 0xFFFFFF00, STS0)
					Or(STS0, 0x0A, STS0)
				}
			}
			Else
			{
				And(STS0, 0xFFFFFF00, STS0)
				Or(STS0, 0x06, STS0)
			}
			Return(Arg3)
		}
	} 

