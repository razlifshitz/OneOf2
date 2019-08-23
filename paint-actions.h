void initDataBeforeFirstRun()
{
    attachServo();
    attachEncoder(&encoder);
    state = BEFORE_DRAWING_MAIN_BRANCH;
    dataCalculated = false;
}

void beforeDrawingMainBranch()
{
    if (not dataCalculated)
    {
        dataCalculated = true;

        // length of waves to skip
        encoderDestination = encoderLocation + EIGHTH_CLICKS_PER_ROUND;
    }

    // move servo to main branch location
    if (myServo.read() != mainBranchLocation)
    {
        myServo.write(mainBranchLocation, DEFAULT_SPEED);
    }

    // moving the encoder 1/8 of plate to skip the part of the waves of the plate
    if (moveEncoder(encoderDestination, RIGHT))
    {
        state = DRAWING_MAIN_BRANCH;

        // pausing machine and waiting for another button press
        pausePaint();
        onFinishAction();
    }
}

void drawMainBranch()
{
    if (not dataCalculated)
    {
        dataCalculated = true;

        // main branch length
        encoderDestination = encoderLocation + EIGHTH_CLICKS_PER_ROUND;
    }

    // painting the main branch by moving the encoder.
    // servo is in place.
    if (moveEncoder(encoderDestination, RIGHT))
    {
        state = CALCULATE_LEAFS_SETTINGS;
        onFinishAction();
    }
}

void calculateLeafsSettings()
{
    // leafs count
    upLeafsCount = CalcRand(MIN_LEAFS, MAX_LEAFS);
    downLeafsCount = CalcRand(MIN_LEAFS, MAX_LEAFS);

    // allocating arrays
    upLeafs[upLeafsCount];
    downLeafs[downLeafsCount];

    // generatings leafs data
    generateLeafs(upLeafs, upLeafsCount, UP);
    generateLeafs(downLeafs, downLeafsCount, DOWN);

    state = MOVING_TO_NEXT_LEAF_CREATION_SPOT;
}

void moveToNextLeafCreationSpot()
{
    if (not dataCalculated)
    {
        dataCalculated = true;

        // counters
        upDrawnLeafs = 0;
        downDrawnLeafs = 0;

        // moving back to to head of the main branch,
        // painting leafs on the way.
        encoderDestination = encoderLocation - EIGHTH_CLICKS_PER_ROUND;
    }

    if (moveEncoder(encoderDestination, LEFT))
    {
        // encoder reached head of main branch
        state = FINISH;
        onFinishAction();
    }
    // encoder reached location to draw leaf (UP)
    else if (hasLeafsToDraw(UP) && encoderLocation <= upLeafs[upDrawnLeafs].creationLocation)
    {
        leafToDraw = upLeafs[upDrawnLeafs];
        state = DRAWING_LEAF_PART_A;
    }
    // encoder reached location to draw leaf (DOWN)
    else if (hasLeafsToDraw(DOWN) && encoderLocation <= downLeafs[downDrawnLeafs].creationLocation)
    {
        leafToDraw = downLeafs[downDrawnLeafs];
        state = DRAWING_LEAF_PART_A;
    }
}

void drawLeafPartA()
{
    if (drawLeaf(leafToDraw.movementA, leafToDraw.direction == UP, RIGHT))
    {
        // Done leaf part A
        state = DRAWING_LEAF_PART_B;
    }
}

void drawLeafPartB()
{
    if (drawLeaf(leafToDraw.movementB, leafToDraw.direction == DOWN, LEFT))
    {
        // Done leaf part B

        // updating the drawn leaf value
        leafToDraw.direction == UP ? upDrawnLeafs++ : downDrawnLeafs++;

        state = MOVING_TO_NEXT_LEAF_CREATION_SPOT;
    }
}