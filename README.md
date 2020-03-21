run the command 'ssh-keygen -o';
save it with name macpro-ub;
Upload to github: curl -u "jainanuj" --data '{"title":"macpro-ub","key":"'"$(cat ~/.ssh/test_key.pub)"'"}' https://api.github.com/user/keys ;
git clone git@github.com:jainanuj/cached_value_iteration.git;
git checkout ubuntuChanges;
move download and install files to the parent folder;
run the install file - 'run_install';
move to the cached value iteration folder;
run tests './run_tests.sh';
#run nocache tests './run_nocache.sh'
move all *perf and op* files to a folder (macproubuntu) and commit.;
