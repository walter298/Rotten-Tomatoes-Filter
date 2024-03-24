import sys
from selenium import webdriver
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

def getRottenTomatoesReviewHTML(rottenTomatoesLink):
    browser = webdriver.Firefox()
    browser.get(rottenTomatoesLink)
    
    #click "load more" until all full webpage is loaded
    while True:
        try:
            load_more_button = WebDriverWait(browser, 10).until(
                EC.element_to_be_clickable((By.CSS_SELECTOR, '[data-qa="load-more-btn"]'))
            )
            load_more_button.click()
        except Exception:
            break
    try:
        wait = WebDriverWait(browser, 10)
        wait.until(EC.invisibility_of_element_located((By.CSS_SELECTOR, '[data-qa="load-more-btn"]')))
        with open("temp_src.txt", "w", encoding="utf-8") as f:
            f.write(browser.page_source)
            f.close()
        print(browser.page_source)
    except Exception as e:
        print(e)
    browser.quit()
   
getRottenTomatoesReviewHTML(sys.argv[1])