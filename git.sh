repo_url= "https://github.com/Slazus/Object_Store.git"

make clean
git add .
git commit -m "$1"
git remote add origin ${repo_url}
git push -u origin master
