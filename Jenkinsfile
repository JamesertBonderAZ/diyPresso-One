pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        // Manual caching of ~/.platformio using stash/unstash (works everywhere, no plugin needed)
        stage('Restore PlatformIO Cache') {
            steps {
                script {
                    try {
                        unstash 'platformio-cache'
                        echo 'PlatformIO cache restored'
                    } catch (err) {
                        echo 'No PlatformIO cache found – will create one after build'
                    }
                }
            }
        }

        stage('Install/Update PlatformIO') {
            steps {
                sh '''
                    python3 -m pip install --upgrade --user platformio
                    export PATH="$HOME/.local/bin:$PATH"
                '''
            }
        }

        stage('Build with PlatformIO') {
            steps {
                sh '''
                    export PATH="$HOME/.local/bin:$PATH"
                    platformio run
                '''
            }
        }

        stage('Save PlatformIO Cache') {
            steps {
                stash includes: '.platformio/', name: 'platformio-cache'
                echo 'PlatformIO cache saved for next build'
            }
        }

        stage('Archive Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/**/firmware.*', 
                                 fingerprint: true, 
                                 onlyIfSuccessful: true
            }
        }
    }

    post {
        always {
            cleanWs()
        }
    }
}
