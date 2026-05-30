@echo off
chcp 65001 > nul
@echo добавляем все в индекс
git add .
@echo пишем коммит
git commit -m "добавил новое"
git push
pause