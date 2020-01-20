
Scope(\_SB.PCI0.I2C3) {
  Device(TCS1)
  {
    Name(_ADR, Zero)
    Name(_HID, "FTSC1000")
    Name(_CID, "PNP0C50")
    Name(_HRV, One)
    Name(_S0W, Zero)
    Name(_DEP, Package(2) {\_SB.GPO0, \_SB.PCI0.I2C3})
    Method(_PS3, 0, Serialized)
    {
    }
    Method(_PS0, 0, Serialized)
    {
    }
    Method(_CRS, 0, NotSerialized)
    {
      Name(RBUF, ResourceTemplate()
      {
        I2CSerialBus(0x0038, ControllerInitiated, 0x00061A80, AddressingMode7Bit, "\\_SB.PCI0.I2C3", 0x00, ResourceConsumer,,)
        GpioInt(Level, ActiveLow, Exclusive, PullDefault, 0x0000, "\\_SB.GPO0", 0x00, ResourceConsumer,,)
        {
          0x0015
        }
        GpioIo(Shared, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly, "\\_SB.GPO0", 0x00, ResourceConsumer,,)
        {
          0x0019
        }
      })
      Return(RBUF)
    }
    Method(_DSM, 4, Serialized)
    {
      //Store ("Method _DSM begin", Debug)
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // DSM Function
        switch(ToInteger(Arg2))
        {
          // Function 0: Query function, return based on revision
          case(0)
          {
            // DSM Revision
            switch(ToInteger(Arg1))
            {
              // Revision 1: Function 1 supported
              case(1)
              {
                //Store ("Method _DSM Function Query", Debug)
                Return(Buffer(One) { 0x03 })
              }

              default
              {
                // Revision 2+: no functions supported
                Return(Buffer(One) { 0x00 })
              }
            }
          }

          // Function 1 : HID Function
          case(1)
          {
            //Store ("Method _DSM Function HID", Debug)
            // HID Descriptor Address
            Return(0x0000)
          }

          default
          {
            // Functions 2+: not supported
            Return(0x0000)
          }
        }
      }
      else
      {
        // No other GUIDs supported
        Return(Buffer(One) { 0x00 })
      }
    }
    Method(_STA, 0, NotSerialized)
    {
//      If(LEqual(TPID, One))
//      {
        Return(0x0F)
//      }
//      Return(Zero)
    }
  }
}