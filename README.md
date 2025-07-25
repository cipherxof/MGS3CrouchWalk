# MGS3CrouchWalk
A plugin that aims to bring crouch walking to Metal Gear Solid 3: Master Collection.

**Download:** [GitHub](https://github.com/cipherxof/MGS3CrouchWalk/releases) | [NexusMods](https://www.nexusmods.com/metalgearsolidmastercollection/mods/118/)

![crouchwalknew](https://github.com/cipherxof/MGS3CrouchWalk/assets/5994581/527a3dc0-a487-4e0b-ac2f-99729e53dcf5)

## Features
- Both slow and fast crouch walking are implemented.
    - NPCs are only alerted while fast crouch walking in close proximity.
- Crouching affects your visibility to enemies (i.e, you can hide behind cover)
- The camo index is dynamically adjusted while crouch walking.
- Seamlessly integrated into the game engine as a genuine feature, without displacing any existing functionality.
  
## Installation
- Extract the contents of the release zip into the game folder.<br />(e.g. "**steamapps\common\MGS3**").

### Steam Deck/Linux Additional Instructions
🚩**You do not need to do this if you are using Windows!**
- Open up the game properties of MGS3 in Steam and add `WINEDLLOVERRIDES="wininet,winhttp=n,b" %command%` to the launch options.

## Credits
- Zoft for mtar research and porting the 3DS animations over to the Master Collection
