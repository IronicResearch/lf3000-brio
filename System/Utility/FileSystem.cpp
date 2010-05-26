#include <StringTypes.h>
#include <Utility.h>
#include <sys/types.h>
#include <dirent.h>

LF_BEGIN_BRIO_NAMESPACE()

//--------------------------------------------------------------------------
Boolean EnumFolder( const CPath& dirIn, tFuncEnumFolder f, tFileSelect type, void* userData)
{
	CPath dir = AppendPathSeparator(dirIn);
	DIR *dirp = opendir(dirIn.c_str());
	if( dirp == NULL )
		return false;

	const CPath	kCurrentDirString = ".";
	const CPath	kParentDirString = "..";
	struct dirent *dp;

	do {
		struct stat filestat;
		if( (dp = readdir(dirp)) != NULL
				&& dp->d_name != kCurrentDirString
				&& dp->d_name != kParentDirString )
		{
			CString theDir = dir + dp->d_name;
			if (stat(theDir.c_str(), &filestat) == 0 && f)
			{
				switch(type) {

				case kFoldersOnly:
					if (filestat.st_mode & S_IFDIR)
						if (!f(theDir, userData))
							goto EXIT;
					break;
				case kFilesOnly:
					if (!(filestat.st_mode & S_IFDIR))
						if (!f(theDir, userData))
							goto EXIT;
					break;
				case kFilesAndFolders:
					if (!f(theDir, userData))
						goto EXIT;
					break;
				default:
					// not sure we want to Assert.
					break;
				}
			}
		}
	} while (dp != NULL);

EXIT:
	closedir(dirp);
	return true;
}

LF_END_BRIO_NAMESPACE()
