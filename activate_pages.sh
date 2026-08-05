#!/bin/bash
# Активация GitHub Pages для thermo-ising-termux через Termux
# Запускать из папки проекта: bash activate_pages.sh

set -e

REPO="andrewrush/thermo-ising-termux"

echo "=== Проверка авторизации gh ==="
gh auth status || { echo "Ошибка: gh не авторизован. Запусти: gh auth login"; exit 1; }

echo ""
echo "=== Проверка видимости репозитория ==="
VISIBILITY=$(gh repo view $REPO --json visibility -q '.visibility')
echo "Visibility: $VISIBILITY"
if [ "$VISIBILITY" != "PUBLIC" ]; then
    echo "Репозиторий приватный. Делаем публичным..."
    gh repo edit $REPO --visibility public
fi

echo ""
echo "=== Проверка текущего статуса Pages ==="
if gh api repos/$REPO/pages >/dev/null 2>&1; then
    echo "Pages уже активированы (возможно, в режиме Actions)."
    echo "Отключаем для чистой перенастройки на branch deploy..."
    gh api repos/$REPO/pages -X DELETE
    sleep 2
else
    echo "Pages ещё не активированы."
fi

echo ""
echo "=== Активация Pages (branch: main, path: /) ==="
echo '{"source":{"branch":"main","path":"/"}}' | gh api repos/$REPO/pages -X POST --input -

echo ""
echo "=== Проверка настроек ==="
sleep 1
gh api repos/$REPO/pages | jq '.html_url, .source' 2>/dev/null || {
    echo "jq не установлен. Установи: pkg install jq -y"
    echo "Или проверь вручную: gh api repos/$REPO/pages"
}

echo ""
echo "================================================"
echo "✅ GitHub Pages активирован!"
echo ""
echo "Через 1–2 минуты демо будет доступно по адресу:"
echo "   https://andrewrush.github.io/thermo-ising-termux/"
echo ""
echo "Если открываешь сразу и видишь 404 — подожди"
echo "ещё минуту или сделай пустой коммит:"
echo "   git commit --allow-empty -m 'trigger: rebuild pages'"
echo "   git push"
echo "================================================"
