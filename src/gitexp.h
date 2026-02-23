#ifndef _GITEXP_H_
#define _GITEXP_H_

#include "virta.h"
extern int GitExport (fs::path RepoPath)
{
    git_libgit2_init();

    git_repository *repo = NULL;
    int error = git_repository_open(&repo, RepoPath);


    git_libgit2_shutdown();

}

#endif //_GITEXP_H_
